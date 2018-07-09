# include <unistd.h>
# include <sys/mman.h>
#include "csapp.h"

int main(int argc,char **argv){
    
    if(argc!=2){
        fprintf(stderr, "usage: %s <file path>\n", argv[0]);
        return 0;
    }
    char * filename = argv[1];
    int fd = open(filename,O_RDONLY, 0);
    struct stat sbuf;
    void *srcp;
    
    if (stat(filename, &sbuf) < 0) {
	    fprintf(stderr, "404,%s Not found", argv[1]);
	    return 0;
    }
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) { //line:netp:doit:readable
	     fprintf(stderr, "403:Forbidden! you couldn't read the file:%s\n", argv[1]);
	    return 0;
	}    

    //这个页面可读，私有对象，写入到
    srcp = mmap(0, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);//line:netp:servestatic:mmap
    write(STDOUT_FILENO,(char *)srcp,sbuf.st_size);
    
    return 0;
}