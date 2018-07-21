#include "csapp.h"
unsigned int snooze1(unsigned int sec){

    unsigned int rest = sleep(sec);
    printf("Slept for %d of %d sec",sec-rest,sec);
    return rest;
}

// int main(int argc,char** argv){
//     if(argc!=2){
//         printf("usage: %s <sleep time>",argv[0]);
//     }else if(atoi(argv[1])<0){
//         printf("please input right time!");
//     }

//     snooze(atoi(argv[1]));
//     return 0;
// }