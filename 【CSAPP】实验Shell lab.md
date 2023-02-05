---
title: 实验一shell_lab
date: 2018-03-20 11:11:54
tags: 
- CSAPP
categories:
- CSAPP系列
---

## 实验内容
就是完成tsh.c里的函数：
1.  eval: Main routine that parses and interprets the command line. [70 lines]
2. builtin_cmd: Recognizes and interprets the built-in commands: quit, fg, bg, and jobs. [25 lines]
3. do bgfg: Implements the bg and fg built-in commands. [50 lines]
4. waitfg: Waits for a foreground job to complete. [20 lines]
5. sigchld handler: Catches SIGCHILD signals. 80 lines]
6. sigint handler: Catches SIGINT (ctrl-c) signals. [15 lines]
7. sigtstp handler: Catches SIGTSTP (ctrl-z) signals. [15 lines]

具体来讲，我们的shell需要提供有三种命令：
+ 一种是bulit-in型，这个就直接在当前线程执行；
+ 一种是执行某个可执行文件，这样就另开一个新线程执行
+ 还有一种是处理信号，比如ctrl-c就让当前进程停止等。

## 思路
一开始有些不知从何下手的感觉，其实我们可以make后用下面两条命令，然后比对输出结果。就可以一步一步的完善我们的shell。

```Bash
./sdriver.pl -t trace01.txt -s ./tshref -a "-p"
./sdriver.pl -t trace01.txt -s ./tsh -a "-p"
```
从trace01到trace16的输出都一致后，我们的shell就完成了。
### 命令解析
这一块其实书上都有，builtin_command扩展一下等等

### 程序执行原理 
就是fork一个子进程，然后父进程获取返回值。子进程执行的时候，父进程是堵塞的。而且要注意fork新进程的过程中要阻塞SIGCHLD信号，原因如下 
 > 在fork()新进程前后要阻塞SIGCHLD信号，防止出现竞争（race）这种经典的同步错误，如果不阻塞可能会出现子进程先结束从jobs中删除，然后再执行到主进程addjob的竞争问题。相关解释和方法见CSAPP P519页。

### 后台执行原理
我们一般在命令行执行命令都是前台执行，也就是会堵塞当前进程的。如果想后台执行（也就是不堵塞当前进程）的话，只需要父进程不再wait子进程的消息。

###  对于builtin_command
+ fg是把某个在后台的停止的程序放到前台执行。怎么做到呢？无非就是发送一个SIGCONT信号，然后wait堵塞住shell进程。SIGCONT的默认处理模式就可以，所以我们不用自己写信号处理程序。
+ bg是把某个后台停止的程序重新在后台执行，也就是发送一个SIGCONT给相关进程。

### 几个细节
1. 当我们使用bulitin_command里的kill 一个job时，最先想到的思路就是调用unix的kill函数，给这个进程发送一个信号，而信号的默认操作就是结束进程。这样固然没有错，但是，考虑下面的情况：

> In this case, the shell forks a child process, then loads and runs the program in the context of the
child. The child processes created as a result of interpreting a single command line are known  collectively as a job. In general, a job can consist of multiple child processes connected by Unix pipes.

要注意一个job不一定只有一个进程，可能也有多个进程。那么kill一个进程就不管用了。最好的方法是为每一个job创建一个进程组，然后直接给kill传递负的进程号，意思是结束该进程号的进程组里的所有进程。
2. 如果我们发送SIGCONT给某个子进程，然后这个进程在wait开始执行之前就执行完了，会怎样？这里就应该听从writeup文件里的建议，waitfg只做sleep处理，不做回收子进程处理。这样的好处就是，waitfg里会少出现一次系统调用错误。
3. fork以后子进程重新设置了进程组PID，但是这种子进程与父进程的关系还在。也就是说，子进程结束以后我们的父进程shell依旧会受到SIGCHLD。
4. 信号处理程序中的函数要是异步信号安全的要么不能被别的信号处理程序中断，要么可重入。产生输出的唯一安全的方法就是使用write。要们就在printf之前把所有信号都屏蔽。
5. 根据安全信号处理的G3，当主程序与信号处理程序同时维护一个全局变量的时候，信号处理程序应该要阻塞所有的信号。那么我们的信号处理程序里删除jobs list里的一项时都需屏蔽所有信号。
6. sigchld_handler函数里，我们对子进程的回收用的函数是waitpid(-1,&status,WNOHANG|WUNTRACED)。而且要用while的原因可以参考[chap8-异常控制流笔记](https://husy1994.github.io/2018/03/17/chap8-%E5%BC%82%E5%B8%B8%E6%8E%A7%E5%88%B6%E6%B5%81/) 

## 待改进&问题
1. 信号处理程序没有做到信号安全，比如用了printf这种函数。虽然考虑到整个测试会比较简单，不太容易出现printf被打断的情况，但是理论上来说需要屏蔽所有信号或者使用一些信号安全的函数。
2. 还有很多细节可以推敲，但是这一阶段求一个整体理解。

## 参考资料
1. [Linux下回收子进程wait函数和waitpid函数的基本使用](https://www.cnblogs.com/yongfengnice/p/6796356.html)
2. [CSAPP: Shell Lab](https://blog.csdn.net/u012336567/article/details/51926577)