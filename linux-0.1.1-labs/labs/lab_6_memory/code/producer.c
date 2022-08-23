#define   __LIBRARY__
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

_syscall2(sem*,sem_open,const char *,name,unsigned int,value);
_syscall1(int,sem_wait,sem*,sem);
_syscall1(int,sem_post,sem*,sem);
_syscall1(int,sem_unlink,const char *,name);
_syscall1(void*,shmat,int,shmid);
_syscall1(int,shmget,char*,name);

#define NUMBER 20 /*打出数字总数*/
#define BUFSIZE 10 /*缓冲区大小*/

sem   *empty, *full, *mutex;

int *buffer;

int init();

void clean();

int main()
{
    int  i;
    int  write_index = 0; /*写入缓冲区位置*/
    
    if(!init())
    {
    	perror("Fail to init!\n");
        return -1;
    }
    
    /*生产者进程*/
    for( i = 0 ; i < NUMBER; i++)
    {
        sem_wait(empty);
        sem_wait(mutex);
        buffer[write_index] = i;
        write_index = ( write_index + 1)% BUFSIZE;
        sem_post(mutex);
        sem_post(full);
    }
    return 0;
}

int init()
{
    int shmid;
    /*打开信号量*/
    if((mutex = sem_open("carpelamutex",1)) == SEM_FAILED)
    {
        perror("sem_open() error!\n");
        return -1;
    }
    if((empty = sem_open("carpelaempty",10)) == SEM_FAILED)
    {
        perror("sem_open() error!\n");
        return -1;
    }
    if((full = sem_open("carpelafull",0)) == SEM_FAILED)
    {
        perror("sem_open() error!\n");
        return -1;
    }
    
    /*打开共享内存*/
    shmid = shmget("buffer");
    if(shmid == -1)
    {
        return -1;
    }
    buffer = (int*) shmat(shmid);
}

void clean()
{
    /*释放信号量*/
    sem_unlink("carpelafull");
    sem_unlink("carpelaempty");
    sem_unlink("carpelamutex");
}
