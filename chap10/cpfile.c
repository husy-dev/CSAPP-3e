#include "csapp.h"

#define DEF_MODE S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define DEF_UMASK S_IWGRP|S_IWOTH
int main(int argc, char **argv) 
{
    int n;
    rio_t rio;
    char buf[MAXLINE];
    char* infile;

    if(argc==2){
        infile = argv[1];
        int fd = open(infile,O_RDONLY,DEF_MODE);
        Rio_readinitb(&rio, fd);
        while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) 
        Rio_writen(STDOUT_FILENO, buf, n);
        exit(0);
    }

    //Rio_readinitb(&rio, STDIN_FILENO);
    Rio_readinitb(&rio, STDOUT_FILENO);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) 
    // Rio_writen(STDOUT_FILENO, buf, n);
	Rio_writen(STDIN_FILENO, buf, n);
    exit(0);
}



