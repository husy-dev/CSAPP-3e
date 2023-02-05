---
title: 【CSAPP】Ch11-网络编程
date: 2018-03-24 11:26:43
tags:
- CSAPP
- 网络
- Network
categories:
- CSAPP系列
---

## 总结
网络服务的提供模型有一个最典型的就是服务器-客户端编程模型。这个不多说，然后是一系列的网络知识，局域网，IP地址这些。具体到我们的操作系统，我们其实主要是要了解：
1. 概念：MIME、HTTP/1.0 GET requests（请求行，请求报头，请求方法等），HTTP/1.0 Response（响应行、响应报头、响应主体、状态码等），CGI。
1. 数据结构：记录IP地址的数据结构，以及其对应的域名数据结构。
2. 网络连接的机制就是通过套接字（更底层的暂时别去管）记住下面的框架：
![unix套接字框架](/Users/husy/Documents/CSAPP-3e/【CSAPP】Ch11-网络编程.assets/Screenshot from 2018-07-25 12-35-56.png)

所以其实最朴素的编程内容无非就是：
1. 建立链接
2. 解析请求

## 笔记
1. unix系统最简单的signal函数。
```C
#include <signal.h>
void (*signal(int signo, void (*func)(int))) (int);
//备注
using F = int(int*, int);//这是一个函数类型
using PF = int(\*)(int\*, int);//这是一个函数指针的类型。
//于是signal其实可以这么看
using A = void(*func)(int)
using B = void (*signal) (int signo,A)
using C = B (int);
```

所以首先，C是一个函数，返回值是一个函数指针，参数是一个int类型。而返回的函数指针的参数又是一个函数指针。而我们要写一个信号处理函数就可以是
```C
void handler(int sig){
 ......
}
//然后在在接受信号比如SIGCHLD的函数里写下这个,
signal(SIGCHLD, handler);
......
```

##家庭作业
11.6 A：Rio\_readlineb里解析了请求行，read\_requesthdrs里解析请求报头。在这两处的读操作后，立马跟一个```Rio_writen(rp->rio_fd, buf, strlen(buf));```
B：由于A的关系，输出就是请求头。当在浏览器里输入：localhost:1234时，输出如下
```
Accepted connection from (localhost, 40620)
GET / HTTP/1.1
Host: localhost:1234
Connection: keep-alive
Cache-Control: max-age=0
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.99 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8
Accept-Encoding: gzip, deflate, br
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-HK;q=0.7,ja-JP;q=0.6,ja;q=0.5
```
因为此时并没有默认的网页返回，因此没有Response headers。
当在浏览器里输入：localhost:1234/index.html时，输出如下

```
Accepted connection from (localhost, 40632)
GET /index.html HTTP/1.1
Host: localhost:1234
Connection: keep-alive
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.99 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8
Accept-Encoding: gzip, deflate, br
Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-HK;q=0.7,ja-JP;q=0.6,ja;q=0.5

Response headers:
HTTP/1.0 200 OK
Server: Tiny Web Server
Connection: close
Content-length: 36
Content-type: text/html

```

首先，端口号变了。虽然我浏览器一直开着，但是由于是重新连接，所以其实是一个新的端口。
C：浏览器版本就是user-Agent的内容```Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.99 Safari/537.36```
D：[详情参考](http://www.infoq.com/cn/news/2014/06/http-11-updated)

11.7 提供视频文件. 这是一个静态的请求，那么我们从serve_static去添加。首先修改get\_filetype里没有解析视频文件后缀的文件类型，比如我们这里解析视频文件的后缀为*.mp4
要注意的是content-type并不是严格按照文件的后缀来决定的，具体的要在官网里查，可以参考[MIME 类型](https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Basics_of_HTTP/MIME_types)，但是有问题。

11.8 就是写一个处理处理函数，等得到SIGCHLD的信号以后再调用wait，这样父进程就不会被阻塞。

## 参考资料：
1. [从浏览器输入一个 url 到页面渲染，涉及的知识点及优化点](https://github.com/sunyongjian/blog/issues/34)