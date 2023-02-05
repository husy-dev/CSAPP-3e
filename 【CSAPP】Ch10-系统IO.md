---
title: 【CSAPP】Ch10-系统IO
date: 2018-03-24 11:26:08
tags:
- CSAPP
- I/O
categories:
- CSAPP系列
---

## 总结
系统I/O是指主存和外部设备之间复制数据的机制，它的重点就在于一个缓冲机制和描述符机制。
1. 描述符机制参考下面的图，这就是整个逻辑，包括共享文件，重定向I/O都是由此展开。
![文件I/O](/Users/husy/Documents/CSAPP-3e/【CSAPP】Ch10-系统IO.assets/Screenshot from 2018-07-23 11-43-35.png)

+ 要注意的是，linux里一切皆文件。因此这里不仅仅指从磁盘读取传统的文件，也包括从网络接口读取数据等

2. 缓冲机制：考虑到从外存读取数据开销大，且速度慢。因此考虑使用缓存使得读取效率高一点。
+ unix提供的系统调用是没有缓存的，C标准库的有缓存。
+ 缓冲机制有：全缓冲（磁盘I/O）、行缓冲（收到一个换行符就输出）、无缓冲（错误输出）

## 笔记
1. RIO I/O比Unix I/O多做了一些什么？
    当直接用Unix I/O的时候，如果读写被中断，读写就中断了，会返回一个错误EINTR,也不管有没有读完。这就很尴尬，因为我们程序并没有问题，中断完了以后我们其实可以继续读写。于是就有了RIO I/O.它引入一个rio_t的结构，帮我们检测是否有读完，没有读完就算遇上了EINTR错误也继续读。 [参考](http://www.cnblogs.com/wzzkaifa/p/7281005.html)
2. RIO包里就包含6个函数：
```C
//无缓冲
ssize_t rio_readn(int fd, void *buf, size_t n);
ssize_t rio_writen(int fd, void *buf, size_t n);

//有缓冲
void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_readlineb(rio_t *rp, void *buf, size_t n);
ssize_t rio_readnb(rio_t *rp, void *buf, size_t n);
//有缓冲内部实现函数
ssize_t rio_read(rio_t *rp,void* userbuf,size_t n);
```
为什么有缓冲里没有write相关的函数呢？因为write不需要缓冲。[同样参考](http://www.cnblogs.com/wzzkaifa/p/7281005.html)

- 我们讲Unix I/O没有缓存，这个是针对应用层面来讲的。它和标准库的区别是，标准库每次调用读写不一定会产生系统调用，而Unix I/O则一定会产生系统调用。参考[Linux 中直接 I/O 机制的介绍](https://www.ibm.com/developerworks/cn/linux/l-cn-directio/)和 [知乎问答](https://www.zhihu.com/question/23349599)

3. 子进程会复制父进程的缓冲区。可以思考一下这个实验[缓冲区问题：子进程会复制父进程的缓冲区](https://blog.csdn.net/damotiansheng/article/details/51992231)

3. 标准I/O的限制：输入函数之后不能紧跟输出函数，输出函数之后也不能紧跟输入函数
    - 因为标准I/O的缓冲区是共用的，比如当前fread操作把缓冲区填满了，文件位置的指针也会偏移一个缓冲区。 然后再立即使用fwrite操作，而此时的缓冲区还有读操作的内容，那么很有可能写操作会把缓冲区读操作的内容写入文件夹。
    - 但是我们可以在中间穿插rewind，fseek，fflush等更新缓冲或者重定义文件位置来处理这种矛盾。但是对于套接字文件，由于我们不能使用lseek，从而就做不到这些了。
