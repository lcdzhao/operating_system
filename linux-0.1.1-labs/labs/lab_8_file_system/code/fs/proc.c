#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/segment.h>
#include <linux/fs.h>
#include <stdarg.h>
#include <unistd.h>

#define set_bit(bitnr,addr) ({ \
register int __res ; \
__asm__("bt %2,%3;setb %%al":"=a" (__res):"a" (0),"r" (bitnr),"m" (*(addr))); \
__res; })

char info_buf[4096] ={'\0'};

extern int vsprintf(char * buf, const char * fmt, va_list args);

int sprintf(char *buf, const char *fmt, ...)
{
	va_list args; int i;
	va_start(args, fmt);
	i=vsprintf(buf, fmt, args);
	va_end(args);
	return i;
}

int get_psinfo()
{
	int read = 0;
	read += sprintf(info_buf+read,"%s","pid\tstate\tfather\tpriority\tcounter\tstart_time\n");
	struct task_struct **p;
	for(p = &FIRST_TASK ; p <= &LAST_TASK ; ++p)
 	if (*p != NULL)
 	{
 		read += sprintf(info_buf+read,"%d\t",(*p)->pid);
 		read += sprintf(info_buf+read,"%d\t",(*p)->state);
 		read += sprintf(info_buf+read,"%d\t",(*p)->father);
		read += sprintf(info_buf+read,"%d\t",(*p)->priority);
 		read += sprintf(info_buf+read,"%d\t",(*p)->counter);
 		read += sprintf(info_buf+read,"%d\n",(*p)->start_time);
 	}
 	return read;
}

int proc_read(int dev, unsigned long * pos, char * buf, int count)
{
	
 	int i;
	if(*pos == 0)
	{
		if(dev == 0)
			get_psinfo();
		else{
			panic("Unknown dev!");
		}
	}
 	for(i=0;i<count;i++)
 	{
 		if(info_buf[i+ *pos ] == '\0')  
          break; 
 		put_fs_byte(info_buf[i+ *pos],buf + i+ *pos);
 	}
 	*pos += i;
 	return i;
}