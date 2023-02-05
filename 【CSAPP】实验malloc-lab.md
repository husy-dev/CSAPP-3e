---
title: 实验malloc_lab
date: 2018-04-29 11:13:35
tags: 
- CSAPP
categories:
- CSAPP系列
---

## 实验内容
这个实验就是要求我们实现自己的malloc和free函数，而不是用c标准库的。但是又不需要你太过于底层和细节，主要是堆内存的管理。至于真正对物理内存的处理，还有地址转换什么的都交给memlib.c里的函数去做。
要完成的函数有：

```C
int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *ptr);
void *mm_realloc(void *ptr, size_t size);
```

## 具体内容
1. memlib.c的作用：不是说要自己实现malloc吗？为什么还要使用mem\_lib的函数？而且mem\_lib里使用了malloc。其实这里我们的实验是要求我们对内存管理有一个实现，重点在于管理。如果完完全全不用malloc的话，我们的mm.c函数还要实现找到合适的物理内存页面，并且做一个维护。而如果我们使用mem\_lib的话，就可以模拟OS的物理页面管理。
3. 具体的实现可以先参考一下课本的mm.c里对空闲块的数据结构
4. 需要注意c/C++不对数组的下表边界做检测的。具体参考[数组的下标越界与内存溢出](https://blog.csdn.net/ljx_5489464/article/details/50314677),[C语言为什么不执行数组边界的有效性检查](https://www.xuebuyuan.com/967089.html)
5. 这里有一个我的误解。我以为不能用结构体。但其实如果直接声明定义结构体，其成员变量是放在栈上的。如果用new的话就是在堆上。所以我们在后面有更复杂的组织方法后可以采用结构体。
6. 隐式空闲链表为什么要引入序言和结尾块？书上说是消除合并时边界条件的技巧。看其在find_fit里的for循环就知道这是为了让这个for循环有一个跳出的条件，以及一个循环开始的初始化条件。
7. 对于实验源代码里的mm.c文件里的几个内存补齐代码，如下
    
```C
#define ALIGNMENT 8 //1
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7) //2
#define SIZE_T_SIZE (ALIGN(sizeof(size_t))) //3
…… 
int newsize = ALIGN(size + SIZE_T_SIZE); //4
```
先参考这个[内存补齐算法](https://blog.csdn.net/jsn_ze/article/details/74011396)能明白1,2。对于3就是一面size\_t的大小不是8的倍数，对于4其实就是把头部的大小算进去以后的对齐。为什么要这样是因为我们在书里看到的32bit长的头部是因为size_t的内部就是一个int，因此块大小就是一个int能表示的大小。这里让这种关系更加抽象，如果我们内部size_t的实现不是一个int，而是一个别的长度的数据类型。我们就需要对这个头部进行内存对齐; 为什么需要单独的先对齐，因为我们规定返回的指针是内存对齐的，而返回的指针不是指向头部，而是指向有效载荷的。
8. 有一个虽然不重要但是特别要注意的是：team的信息一定要全部填好，不然会出错。

## 思路
+ 因为不能题目要求不能新建结构体等复杂的数据结构，于是对这个内存的管理全靠地址去推。我觉得直接用隐式空闲链表，也许会不错，先来试一下：
![堆块的结构](http://static.zybuluo.com/Husy/m3z21teg443nvmplvlihmvbp/Screenshot%20from%202018-07-22%2015-15-00.png)
>Since the libc malloc always returns payload pointers that are aligned to 8 bytes, your malloc implementation should do likewise and always return 8-byte aligned pointers.
注意，书上是边界对齐。也就是块头部的地址是8的倍数就好。但是这里要求的是指向有效载荷起始地址的指针地址是8的倍数（虽然其实是一样的）。

+ 然后考虑mm\_malloc的实现，获取到的size首先要对齐，把它变成最小的8的倍数。然后加上头部计算出最后需要的内存大小newSize，然后mem\_sbrk申请； 然后把头部写进去。头部地址+4就是有效载荷起始地址。

## 注意的细节
1. 获取到有效载荷的指针p，p-4就该块的头部，将其视作一个int，然后修改最低位的分配1为0.
2. 为了空间利用率高，我们肯定还需要合并空闲块。这里就需要看前后的块。但是显然这一种只有头部的结构看当前块的后一块是很方便的，看前一块就不方便了。因此按照书上的引入边界标记，引入脚部。只有头部可以类比于单链表，有了脚部就可以算作是双链表。
    
## 改进
1. 线程安全的动态分配

## 参考资料
1. [什么是计算机的大小端规则？](https://blog.csdn.net/github_35681219/article/details/52743048)
2. [【不周山之读厚 CSAPP】VI Malloc Lab](https://wdxtub.com/2016/04/16/thick-csapp-lab-6/)
3. [chap9-虚拟内存](https://husy1994.github.io/2018/05/01/chap9-%E8%99%9A%E6%8B%9F%E5%86%85%E5%AD%98/)