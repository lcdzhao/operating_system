## 课程说明
本实验是 操作系统之基础 - 网易云课堂 的配套实验，推荐大家进行实验之前先学习相关课程：
- L4 操作系统接口
- L5 系统调用的实现
> Tips：点击上方文字中的超链接或者输入 https://mooc.study.163.com/course/1000002004#/info 进入理论课程的学习。 如果网易云上的课程无法查看，也可以看 Bilibili 上的 操作系统哈尔滨工业大学李治军老师。

## 实验目的
- 建立对系统调用接口的深入认识；
- 掌握系统调用的基本过程；
- 能完成系统调用的全面控制；
- 为后续实验做准备。
## 实验内容
此次实验的基本内容是：在 Linux 0.11 上添加两个系统调用，并编写两个简单的应用程序测试它们。

#### （1）iam()
第一个系统调用是 iam()，其原型为：
```
int iam(const char * name);
```
完成的功能是将字符串参数 name 的内容拷贝到内核中保存下来。要求 name 的长度不能超过 23 个字符。返回值是拷贝的字符数。如果 name 的字符个数超过了 23，则返回 “-1”，并置 errno 为 EINVAL。

在 kernal/who.c 中实现此系统调用。

#### （2）whoami()
第二个系统调用是 whoami()，其原型为：
```
int whoami(char* name, unsigned int size);
```
它将内核中由 iam() 保存的名字拷贝到 name 指向的用户地址空间中，同时确保不会对 name 越界访存（name 的大小由 size 说明）。返回值是拷贝的字符数。如果 size 小于需要的空间，则返回“-1”，并置 errno 为 EINVAL。

也是在 kernal/who.c 中实现。

#### （3）测试程序
运行添加过新系统调用的 Linux 0.11，在其环境下编写两个测试程序 iam.c 和 whoami.c。最终的运行结果是：
```
$ ./iam lizhijun

$ ./whoami

lizhijun
```

## 实验提示
首先，请将 Linux 0.11 的源代码恢复到原始状态。
```
# 删除原来的文件
$ cd ~/oslab
$ sudo rm -rf ./*

# 重新拷贝
$ cp -r /home/teacher/oslab/* ./
```
操作系统实现系统调用的基本过程（在 MOOC 课程中已经给出了详细的讲解）是：

- 应用程序调用库函数（API）；
- API 将系统调用号存入 EAX，然后通过中断调用使系统进入内核态；
- 内核中的中断处理函数根据系统调用号，调用对应的内核函数（系统调用）；
- 系统调用完成相应功能，将返回值存入 EAX，返回到中断处理函数；
- 中断处理函数返回到 API 中；
- API 将 EAX 返回给应用程序。

### 应用程序如何调用系统调用
在通常情况下，调用系统调用和调用一个普通的自定义函数在代码上并没有什么区别，但调用后发生的事情有很大不同。

调用自定义函数是通过 call 指令直接跳转到该函数的地址，继续运行。

而调用系统调用，是调用系统库中为该系统调用编写的一个接口函数，叫 API（Application Programming Interface）。API 并不能完成系统调用的真正功能，它要做的是去调用真正的系统调用，过程是：

- 把系统调用的编号存入 EAX；
- 把函数参数存入其它通用寄存器；
- 触发 0x80 号中断（int 0x80）。
linux-0.11 的 lib 目录下有一些已经实现的 API。Linus 编写它们的原因是在内核加载完毕后，会切换到用户模式下，做一些初始化工作，然后启动 shell。而用户模式下的很多工作需要依赖一些系统调用才能完成，因此在内核中实现了这些系统调用的 API。

>后面的目录如果没有特殊说明，都是指在 /home/shiyanlou/oslab/linux-0.11 中。比如下面的 lib/close.c，是指 /home/shiyanlou/oslab/linux-0.11/lib/close.c。

我们不妨看看 lib/close.c，研究一下 close() 的 API：
```
#define __LIBRARY__
#include <unistd.h>

_syscall1(int, close, int, fd)
```
其中 _syscall1 是一个宏，在 include/unistd.h 中定义。
```
#define _syscall1(type,name,atype,a) \
type name(atype a) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
    : "=a" (__res) \
    : "0" (__NR_##name),"b" ((long)(a))); \
if (__res >= 0) \
    return (type) __res; \
errno = -__res; \
return -1; \
}
```
将 _syscall1(int,close,int,fd) 进行宏展开，可以得到：
```
int close(int fd)
{
    long __res;
    __asm__ volatile ("int $0x80"
        : "=a" (__res)
        : "0" (__NR_close),"b" ((long)(fd)));
    if (__res >= 0)
        return (int) __res;
    errno = -__res;
    return -1;
}
```
这就是 API 的定义。它先将宏 \__NR_close 存入 EAX，将参数 fd 存入 EBX，然后进行 0x80 中断调用。调用返回后，从 EAX 取出返回值，存入 __res，再通过对 __res 的判断决定传给 API 的调用者什么样的返回值。

其中 \__NR_close 就是系统调用的编号，在 include/unistd.h 中定义：
```
#define __NR_close    6
/*
所以添加系统调用时需要修改include/unistd.h文件，
使其包含__NR_whoami和__NR_iam。
*/
```

```
/*
而在应用程序中，要有：
*/

/* 有它，_syscall1 等才有效。详见unistd.h */
#define __LIBRARY__

/* 有它，编译器才能获知自定义的系统调用的编号 */
#include "unistd.h"

/* iam()在用户空间的接口函数 */
_syscall1(int, iam, const char*, name);

/* whoami()在用户空间的接口函数 */
_syscall2(int, whoami,char*,name,unsigned int,size);
```
在 0.11 环境下编译 C 程序，包含的头文件都在 /usr/include 目录下。

该目录下的 unistd.h 是标准头文件（它和 0.11 源码树中的 unistd.h 并不是同一个文件，虽然内容可能相同），没有 \__NR_whoami 和 \__NR_iam 两个宏，需要手工加上它们，也可以直接从修改过的 0.11 源码树中拷贝新的 unistd.h 过来。

