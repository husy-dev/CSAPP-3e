#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
int isRequestHeader(buf);
int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    //处理命令行
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    //接收连接请求
    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
            Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                        port, MAXLINE, 0);
            printf("Accepted connection from (%s, %s)\n", hostname, port);
        //开始处理http request。
        doit(connfd); 
        //这里我们做完了就关闭连接                                            
        Close(connfd);                                          
    }
    //不会执行到这 
    return 0;
}

/*
* 主要功能就是从描述符中获取http request，并解析它的请求是什么（是GET/POST？hostname）
*/
void doit(int fd) 
{
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE]; 
    rio_t rio;
    char endserver_http_header[MAXLINE];
    char hostname[MAXLINE],path[MAXLINE];
    int port;
    int end_serverfd;

    //初始化读取
    Rio_readinitb(&rio, fd);
     //读请求行
    if (!Rio_readlineb(&rio, buf, MAXLINE)) 
        return;     
    sscanf(buf, "%s %s %s", method, uri, version);       //解析请求行
    if (strcasecmp(method, "GET")) {                     //暂时只支持GET方法
        clienterror(fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }


    //解析uri的结构
    parse_uri(uri,hostname,path);
    //开始构建要发送给服务器的请求
    build_http_header(endserver_http_header,hostname,path,port,&rio);

    /*connect to the end server*/
    end_serverfd = connect_endServer(hostname,port,endserver_http_header);
    if(end_serverfd<0){
        printf("connection failed\n");
        return;
    }
}

//解析uri
int parse_uri(char *uri, char *filename, char *cgiargs) 
{
    char *ptr;

    if (!strstr(uri, "cgi-bin")) {  /* Static content */ //line:netp:parseuri:isstatic
	strcpy(cgiargs, "");                             //line:netp:parseuri:clearcgi
	strcpy(filename, ".");                           //line:netp:parseuri:beginconvert1
	strcat(filename, uri);                           //line:netp:parseuri:endconvert1
	if (uri[strlen(uri)-1] == '/')                   //line:netp:parseuri:slashcheck
	    strcat(filename, "home.html");               //line:netp:parseuri:appenddefault
	return 1;
    }
    else {  /* Dynamic content */                        //line:netp:parseuri:isdynamic
	ptr = index(uri, '?');                           //line:netp:parseuri:beginextract
	if (ptr) {
	    strcpy(cgiargs, ptr+1);
	    *ptr = '\0';
	}
	else 
	    strcpy(cgiargs, "");                         //line:netp:parseuri:endextract
	strcpy(filename, ".");                           //line:netp:parseuri:beginconvert2
	strcat(filename, uri);                           //line:netp:parseuri:endconvert2
	return 0;
    }
}

void build_http_header(char* endserver_http_header,char* hostname,char* path,int port,rio_t *rio){
    char buf[MAXLINE],request_hdr[MAXLINE],other_hdr[MAXLINE],host_hdr[MAXLINE];

    sprintf(request_hdr,"%s %s %s",);
    /*get other request header for client rio and change it */
    while(Rio_readlineb(client_rio,buf,MAXLINE)>0)
    {
        if(strcmp(buf,endof_hdr)==0) break;/*EOF*/

        if(!strncasecmp(buf,host_key,strlen(host_key)))/*Host:*/
        {
            strcpy(host_hdr,buf);
            continue;
        }

        if(!strncasecmp(buf,connection_key,strlen(connection_key))
                &&!strncasecmp(buf,proxy_connection_key,strlen(proxy_connection_key))
                &&!strncasecmp(buf,user_agent_key,strlen(user_agent_key)))
        {
            strcat(other_hdr,buf);
        }
    }
    if(strlen(host_hdr)==0)
    {
        sprintf(host_hdr,host_hdr_format,hostname);
    }
    sprintf(http_header,"%s%s%s%s%s%s%s",
            request_hdr,
            host_hdr,
            conn_hdr,
            prox_hdr,
            user_agent_hdr,
            other_hdr,
            endof_hdr);

    return ;

}