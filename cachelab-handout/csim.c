#include "cachelab.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>

typedef struct
{
    unsigned long int valid;
    unsigned long int tag;

} cache_line;

typedef struct
{
    unsigned long int lines;    //有多少行
    unsigned long int set_bits; //相当于缓存有2^set_bits个组
    unsigned long block_bits;
} cache_format;

typedef struct
{
    unsigned long int tag;
    unsigned long int set_index;
} addrInfo;

typedef struct
{
    unsigned int hits;
    unsigned int misses;
    unsigned int evictions;
} statInfo;

statInfo statinfo = {0, 0, 0};
cache_format format = {0, 0, 0};

void print_help_info(void);
//解析一个地址，返回-1表示地址格式错误，一般返回1。
int parseAddr(unsigned int addr, addrInfo *addrinfo);
//通过tag和set返回是否命中，0=未命中，1=命中
int inCachetable(cache_line *cache, addrInfo *addrinfo);
//将line更新为最近读取
void refreshCache(cache_line *cache, int index,addrInfo *addrinfo);
//将这个tag和set写入cache表中,需要处理换出的情况
void writeCache(cache_line *cache, addrInfo *addrinfo);
//初始化cache表
cache_line *initCacheTable();
//检查命令的有效性
void checkCommand();
//统计hit，miss和eviction次数
void statTools(char *fileName, cache_line *cache, int visible);
void do_L(unsigned int addr, addrInfo *addrinfo, cache_line *cache);
void do_M(unsigned int addr, addrInfo *addrinfo, cache_line *cache);
void do_S(unsigned int addr, addrInfo *addrinfo, cache_line *cache);
void do_L_visible(unsigned int addr, addrInfo *addrinfo, cache_line *cache);
void do_S_visible(unsigned int addr, addrInfo *addrinfo, cache_line *cache);
void do_M_visible(unsigned int addr, addrInfo *addrinfo, cache_line *cache);

int main(int argc, char **argv)
{
    // 解析命令行参数
    int opt, help = 0, visible = 0;
    char *fileName;
    // cache_format *format = (cache_format *)malloc(sizeof(cache_format));
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            help = 1;
            break;
        case 'v':
            visible = 1;
            break;
        case 's':
            format.set_bits = atoi(optarg);
            break;
        case 'E':
            format.lines = atoi(optarg);
            break;
        case 'b':
            format.block_bits = atoi(optarg);
            break;
        case 't':
            fileName = optarg;
            break;
        default:
            help = 1;
            break;
        }
    }

    //如果只是需要打印帮助信息
    if (help == 1)
    {
        print_help_info();
        exit(0);
    }
    //检查命令行的合法性
    checkCommand();
    //开始构建cache表
    cache_line *cache = initCacheTable();
    //开始读取文件并解析
    statTools(fileName, cache, visible);
    //读完了，释放内存空间
    free(cache);
    //打印
    printSummary(statinfo.hits, statinfo.misses, statinfo.evictions);
    return 0;
}
//检查命令的有效性
void checkCommand()
{
    /*检查缓存的设置，比如
    *1. set，line，block的长度是否是数字且大于0
    *2. fileName是否存在，且可以打开。  
    */
    if (format.set_bits == 0 || format.lines == 0 || format.block_bits == 0)
    {
        printf("Arguments Error!\n\n");
        print_help_info();
        exit(1);
    }
}

//初始化cache表
cache_line *initCacheTable()
{
    int set_num = 2 << (format.set_bits);
    cache_line *cache = (cache_line *)malloc(sizeof(cache_line) * set_num * (format.lines));
    if (cache == NULL)
    {
        printf("Memory Error!\n");
        exit(1);
    }
    //malloc是没有初始化的，这里要初始化表
    for (int index = 0; index < (set_num * format.lines); index++)
    {
        (cache + index)->valid = 0;
        (cache + index)->tag = 0xffffffff;
    }
    return cache;
}

void print_help_info(void)
{
    // Print the help inforation of this program
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\n");
    printf("Examples:\n");
    printf("  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

int parseAddr(unsigned int addr, addrInfo *addrinfo)
{

    addrinfo->tag = addr >> (format.block_bits + format.set_bits);
    addrinfo->set_index = (addr >> format.block_bits) & ~(~0 << format.set_bits);
    return 1;
}

//如果某line命中，将line更新为最近读取,index是要更新的在cache里的序号，另外，这里并没有删除任何数据，因此也不许要free
void refreshCache(cache_line *cache, int index,addrInfo *addrinfo)
{
    cache_line* tmp = (cache_line*)malloc(sizeof(cache_line));
    memcpy(tmp,&cache[index],sizeof(cache_line));
    int end = (addrinfo->set_index+1)*format.lines;
    for(int i=index+1;i<end;++i){
        memcpy(&cache[i-1], &cache[i], sizeof(cache_line));
    }
    memcpy(&cache[end-1],tmp,sizeof (cache_line));
    free(tmp);
}

//是否命中，0=未命中，1=命中
int inCachetable(cache_line *cache, addrInfo *addrinfo)
{
    //这里我们的set是从0开始的,cache+(set*lines)指向当前set的第一个line
    // cache_line *p = cache + (addrinfo->set_index) * (format.lines);
    int start = (addrinfo->set_index) * (format.lines);
    int end = start+format.lines;
    for (int i = start; i < end; ++i)
    {
        if (cache[i].tag == addrinfo->tag && (cache[i].valid == 1))
        {
            refreshCache(cache, i,addrinfo);
            return 1;
        }
    }
    return 0;
}

//将这个tag和set写入cache表中,需要处理换出的情况
void writeCache(cache_line *cache, addrInfo *addrinfo)
{
    //判断是否是满的，就是扫描一遍set
    int start = (addrinfo->set_index) * (format.lines);
    int end = start+format.lines;
    int flag = 1;
    for (int i = start; i < end; ++i)
    {
        if (cache[i].valid == 0)
        {
            cache[i].tag = addrinfo->tag;
            cache[i].valid = 1;
            flag = 0;
            refreshCache(cache, i,addrinfo);
            break;
        }
    }
    //如果没找到可以写的，就需要换出算法，也就是把这一组的第一个换出来,也就是直接覆盖掉第一个cache_line.因此loc和start都不用更新
    if (flag)
    {
        cache[start].tag = addrinfo->tag;
        refreshCache(cache, start,addrinfo);
        ++statinfo.evictions;
        printf(" eviction");
    }
}
void statTools(char *fileName, cache_line *cache, int visible)
{
    // statInfo *statinfo = (statInfo *)malloc(sizeof(statInfo));
    FILE *fp = fopen(fileName, "r");
    char operation;
    unsigned int addr;
    int dataSize;
    addrInfo *addrinfo = (addrInfo *)malloc(sizeof(addrInfo));
    if (visible == 1)
    {

        while (fscanf(fp, " %c %x,%d", &operation, &addr, &dataSize) > 0)
        {
            printf("%c %x,%d", operation, addr, dataSize);
            switch (operation)
            {
            case 'I':
                break;
            case 'L':
                do_L_visible(addr, addrinfo, cache);
                break;
            case 'S':
                do_S_visible(addr, addrinfo, cache);
                break;
            case 'M':
                do_M_visible(addr, addrinfo, cache);
                break;
            }
            printf("\n");
        }
    }
    else
    {
        // addrInfo* addrinfo = (addrInfo *)malloc(sizeof (addrInfo));
        while (fscanf(fp, " %c %x,%d", &operation, &addr, &dataSize) > 0)
        {
            switch (operation)
            {
            case 'I':
                break;
            case 'L':
                do_L(addr, addrinfo, cache);
                break;
            case 'S':
               do_S(addr, addrinfo, cache);
                break;
            case 'M':
                do_M(addr, addrinfo, cache);
                break;
            }
        }
    }
    free(addrinfo);
    fclose(fp);
}

void do_L(unsigned int addr, addrInfo *addrinfo, cache_line *cache)
{
    parseAddr(addr, addrinfo);
    if (!inCachetable(cache, addrinfo))
    {
        writeCache(cache, addrinfo); //这个里面用来判断是否需要换出操作，并对EVICTION操作
        ++statinfo.misses;
    }
    else
    {
        ++statinfo.hits;
    }
}

void do_L_visible(unsigned int addr, addrInfo *addrinfo, cache_line *cache)
{
    parseAddr(addr, addrinfo);
    if (!inCachetable(cache, addrinfo))
    {
        ++statinfo.misses;
        printf(" miss");
        writeCache(cache, addrinfo); //这个里面用来判断是否需要换出操作，并对EVICTION操作
    }
    else
    {
        ++statinfo.hits;
        printf(" hits");
    }
}

void do_S(unsigned int addr, addrInfo *addrinfo, cache_line *cache)
{
    do_L(addr, addrinfo, cache);
}

void do_S_visible(unsigned int addr, addrInfo *addrinfo, cache_line *cache)
{
    do_L_visible(addr, addrinfo, cache);
}
void do_M(unsigned int addr, addrInfo *addrinfo, cache_line *cache)
{
    parseAddr(addr, addrinfo);
    if (!inCachetable(cache, addrinfo))
    {
        ++statinfo.misses;
        writeCache(cache, addrinfo); //这个里面用来判断是否需要换出操作，并对EVICTION操作
    }
    else
    {
        ++statinfo.hits;
    }
    ++statinfo.hits;
}

void do_M_visible(unsigned int addr, addrInfo *addrinfo, cache_line *cache)
{
    parseAddr(addr, addrinfo);
    if (!inCachetable(cache, addrinfo))
    {
        ++statinfo.misses;
        printf(" miss");
        writeCache(cache, addrinfo); //这个里面用来判断是否需要换出操作，并对EVICTION操作
    }
    else
    {
        ++statinfo.hits;
        printf(" hits");
    }
    ++statinfo.hits;
    printf(" hits");
}