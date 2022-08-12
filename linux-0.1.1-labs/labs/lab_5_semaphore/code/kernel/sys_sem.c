#define __LIBRARY__  
#include <unistd.h>  
#include <linux/sched.h>  
#include <linux/kernel.h>  
#include <asm/segment.h>  
#include <asm/system.h>   

#define SEM_COUNT 5 
sem semaphores[SEM_COUNT] ={
	{0, 0, "NULL", NULL},
	{0, 0, "NULL", NULL},
	{0, 0, "NULL", NULL},
	{0, 0, "NULL", NULL},
	{0, 0, "NULL", NULL}
}; 

void init_queue(t_queue* q)  
{  
    q->front = q->rear = 0; 
}

int is_empty(t_queue* q)
{
	return q->front == q->rear?1:0;
}

int is_full(t_queue* q)
{    
	return (q->rear+1)%QUE_LEN == q->front?1:0;
}

struct task_struct * get_first__task(t_queue* q)
{
	if(is_empty(q)) 
	{
	//	printk("Queue is empty!\n");
		return NULL;
	}
	struct task_struct *tmp = q->wait_tasks[q->front]; 
	q->front = (q->front+1)%QUE_LEN;
	return tmp;
}

int insert_task(struct task_struct *p,t_queue* q)
{
	printk("Insert %d",p->pid);
	if(is_full(q))
	{
		printk("Queue is full!\n");
		return -1;
	}
	q->wait_tasks[q->rear] = p;
	q->rear = (q->rear+1)%QUE_LEN;
	return 1;
}

int sem_location(const char* name)
{  
    int i;
    for(i = 0;i < SEM_COUNT; i++)  
    {  
        if(strcmp(name,semaphores[i].name) == 0 && semaphores[i].occupied == 1) 
        {     
            return i;  
        }  
    }  
    return -1; 
}  

sem* sys_sem_open(const char* name,unsigned int value)
{
	char tmp[16];
	char c;
	int i;
	for( i = 0; i<16; i++)
	{
		c = get_fs_byte(name+i);
		tmp[i] = c;
		if(c =='\0') break;
	}
	if(c >= 16) 
	{ 	
		printk("Semaphore name is too long!");
		return NULL;
	}
	if((i = sem_location(tmp)) != -1)
	{
		return &semaphores[i];
	}
	for(i = 0;i< SEM_COUNT; i++)
	{
		if(!semaphores[i].occupied)
		{
			strcpy(semaphores[i].name,tmp);
			semaphores[i].occupied = 1;
			semaphores[i].value = value;
			init_queue(&(semaphores[i].wait_queue));
			printk("%d %d %d %s\n",semaphores[i].occupied,i,semaphores[i].value,semaphores[i].name);
			printk("%p\n",&semaphores[i]); 
			return &semaphores[i];
		}
	}	
	printk("Numbers of semaphores are limited!\n");
	return NULL;
}  

int sys_sem_wait(sem* sem)
{
	cli();
	sem->value--;
	sti();
	if(sem->value < 0)
	{
		// 参见sleep_on
		current->state = TASK_UNINTERRUPTIBLE;
		insert_task(current,&(sem->wait_queue));
		schedule();
	}
	return 0;
}

int sys_sem_post(sem* sem)
{
	cli();
	struct task_struct *p;
	sem->value++;
	sti();
	if(sem->value <= 0)
	{
		p = get_first__task(&(sem->wait_queue));
		if(p != NULL)
		{
			(*p).state = TASK_RUNNING;
		}
	}
	return 0;
}

int sys_sem_unlink(const char *name)  
{  
    char tmp[16];
    char c;
	int i;
	for( i = 0; i<16; i++)
	{
		c = get_fs_byte(name+i);
		tmp[i] = c;
		if(c =='\0') break;
	}
	if(c >= 16) 
	{
		printk("Semphore name is too long!");
		return -1;
	}
    int ret = sem_location(tmp); 
    if(ret != -1)
    {
    	semaphores[ret].value = 0;
    	strcpy(semaphores[ret].name,"\0");
    	semaphores[ret].occupied = 0;
    	return 0;
    }   
    return -1;  
}  
