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

#define NUMBER 10 /*打出数字总数*/
#define CHILD 5 /*消费者进程数*/
#define BUFSIZE 10 /*缓冲区大小*/

sem *empty, *full, *mutex;
int fno; /*文件描述符*/

int init();

void producer_write_num(int index,int num);

int comsumer_read_num();

int main()
{
    int i = 0;
    int j = 0;
    int k = 0;
    int write_index = 0;
    pid_t p;
    if(!init()){
        return -1;
    }

    if((p = fork()) == 0)
    {
        /*process of producer*/
        for(i = 0 ; i < NUMBER; i++)
        {
            producer_write_num(write_index, i);
            write_index = ( write_index + 1)% BUFSIZE;
        }
        return 0;
    }else if(p < 0)
    {
        perror("Fail to fork producer!\n");
        return -1;
    }

    for(j = 0; j < CHILD ; j++)
    {
        if((p=fork())==0)
        {
            /*process of consumer*/
            for(k = 0; k < NUMBER/CHILD; k++)
            {
                printf("%d:  %d\n",getpid(),comsumer_read_num());
                fflush(stdout);
            }
           return 0;
        }else if(p < 0)
        {
            perror("Fail to fork consumer!\n");
            return -1;
        }
    }
    /*wait for childs*/
    wait(NULL);
   
    sem_unlink("carfull");
    sem_unlink("carempty");
    sem_unlink("carmutex");
  
    close(fno);
    return 0;
}

int init() {
    int inited_read_index = 0;
    
    if((mutex = sem_open("carmutex",1)) == SEM_FAILED || mutex = -1)
    {
        perror("sem_open() carmutex error!\n");
        return -1;
    }
    if((empty = sem_open("carempty",10)) == SEM_FAILED || empty = -1)
    {
        perror("sem_open() carempty error!\n");
        return -1;
    }
    if((full = sem_open("carfull",0)) == SEM_FAILED || full==-1)
    {
        perror("sem_open() carfull error!\n");
        return -1;
    }
    if((fno = open("buffer.dat",O_CREAT|O_RDWR|O_TRUNC,0666)) == -1)
    {
        perror("open file(buffer.dat) error!\n");
        return -1;
    }
    
    /*write read_index into buffer to sync read_index in multiple processes of childs*/
    lseek(fno,10*sizeof(int),SEEK_SET);
    write(fno,(char *)&inited_read_index,sizeof(int));
    return 1;
}

void producer_write_num(int index,int num){
    printf("producer: write_index:%d,num:%d\n",index,num);
    fflush(stdout);
    
    sem_wait(empty);
    sem_wait(mutex);
   
    lseek(fno, index*sizeof(int), SEEK_SET); 
    write(fno,(char *)&num,sizeof(int));  

    sem_post(mutex);
    sem_post(full);
}

int comsumer_read_num(){
    int read_index;
    int readed;
    sem_wait(full);
    sem_wait(mutex);

    /*read read_index*/
    lseek(fno,10*sizeof(int),SEEK_SET);
    read(fno,(char *)&read_index,sizeof(int));

    /*read num*/
    lseek(fno,read_index*sizeof(int),SEEK_SET);
    read(fno,(char *)&readed,sizeof(int));
    
    printf("comsumer: read_index:%d,num:%d\n",read_index,readed);
    fflush(stdout);

    /*write read_index into buffer to sync read_index in multiple processes of childs*/
    read_index = (read_index + 1) % BUFSIZE;
    lseek(fno,10*sizeof(int),SEEK_SET);
    write(fno,(char *)&read_index,sizeof(int));

    sem_post(mutex);
    sem_post(empty);

    return readed;
}
