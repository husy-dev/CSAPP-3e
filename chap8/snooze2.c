#include "csapp.h"
extern unsigned int snooze1(unsigned int sec);

void sigint_handler(int sig){
    //    exit(0);
    return;
}

int main(int argc,char**argv){
    if(argc!=2){
        printf("usage: %s <sleep time>",argv[0]);
    }else if(atoi(argv[1])<0){
        printf("please input right time!");
    }

    if(signal(SIGINT,sigint_handler)==SIG_ERR){
        unix_error("signal_error!");
    }

    int rest = snooze1(atoi(argv[1]));
    return 0;
}