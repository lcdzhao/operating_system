## 实验目的
- 熟悉 hit-oslab 实验环境；
- 建立对操作系统引导过程的深入认识；
- 掌握操作系统的基本开发过程；
- 能对操作系统代码进行简单的控制，揭开操作系统的神秘面纱。
## 实验内容
此次实验的基本内容是：

阅读《Linux 内核完全注释》的第 6 章，对计算机和 Linux 0.11 的引导过程进行初步的了解；
- 按照下面的要求改写 0.11 的引导程序 bootsect.s
- 有兴趣同学可以做做进入保护模式前的设置程序 setup.s。

#### 改写 bootsect.s 主要完成如下功能：
bootsect.s 能在屏幕上打印一段提示信息“XXX is booting...”，其中 XXX 是你给自己的操作系统起的名字，例如 LZJos、Sunix 等（可以上论坛上秀秀谁的 OS 名字最帅，也可以显示一个特色 logo，以表示自己操作系统的与众不同。）

#### 改写 setup.s 主要完成如下功能：
- bootsect.s 能完成 setup.s 的载入，并跳转到 setup.s 开始地址执行。而 setup.s 向屏幕输出一行"Now we are in SETUP"。
- setup.s 能获取至少一个基本的硬件参数（如内存参数、显卡参数、硬盘参数等），将其存放在内存的特定地址，并输出到屏幕上。
- setup.s 不再加载 Linux 内核，保持上述信息显示在屏幕上即可。

## 实验提示
操作系统的 boot 代码有很多，并且大部分是相似的。本实验仿照 Linux-0.11/boot 目录下的 bootsect.s 和 setup.s，以剪裁它们为主线。当然，如果能完全从头编写，并实现实验所要求的功能，是再好不过了。

同济大学赵炯博士的《Linux 内核 0.11 完全注释（修正版 V3.0）》（以后简称《注释》）的第 6 章是非常有帮助的参考，实验中可能遇到的各种问题，几乎都能找到答案。谢煜波撰写的《操作系统引导探究》也是一份很好的参考。

需要注意的是，oslab 中的汇编代码使用 as86 编译。

下面将给出一些更具体的 “提示”。这些提示并不是实验的一步一步的指导，而是罗列了一些实验中可能遇到的困难，并给予相关提示。它们肯定不会涵盖所有问题，也不保证其中的每个字都对完成实验有帮助。所以，它们更适合在你遇到问题时查阅，而不是当作指南一样地亦步亦趋。本课程所有实验的提示都是秉承这个思想编写的。

## 开始实验前
在正式开始实验之前，你需要先了解下面的内容：

#### （1）相关代码文件
Linux 0.11 文件夹中的 boot/bootsect.s、boot/setup.s 和 tools/build.c 是本实验会涉及到的源文件。它们的功能详见《注释》的 6.2、6.3 节和 16 章。

#### （2）引导程序的运行环境
引导程序由 BIOS 加载并运行。它活动时，操作系统还不存在，整台计算机的所有资源都由它掌控，而能利用的功能只有 BIOS 中断调用。

实验中主要使用 BIOS 0x10(显示) 和 0x13(读磁盘) 中断。

## 完成 bootsect.s 的屏幕输出功能
> 代码中以 ! 开头的行都是注释，实际在写代码时可以忽略。
> 实验中所有提到的修改，均是指相对于 linux-0.11 中的代码。

首先来看完成屏幕显示的关键代码，如下：
```
! 首先读入光标位置
    mov ah,#0x03
    xor bh,bh
    int 0x10

! 显示字符串 “Hello OS world, my name is LZJ”
! 要显示的字符串长度
    mov cx,#36
    mov bx,#0x0007
    mov bp,#msg1
! es:bp 是显示字符串的地址
! 相比与 linux-0.11 中的代码，需要增加对 es 的处理，因为原代码中在输出之前已经处理了 es
    mov ax,#0x07c0
    mov es,ax
    mov ax,#0x1301
    int 0x10

! 设置一个无限循环
inf_loop:
    jmp inf_loop
copy
```
这里需要修改的是字符串长度，即用需要输出的字符串长度替换 mov cx,#24 中的 24。要注意：除了我们设置的字符串 msg1 之外，还有三个换行 + 回车，一共是 6 个字符。比如这里 Hello OS world, my name is LZJ 的长度是 30，加上 6 后是 36，所以代码应该修改为 mov cx,#36。

接下来就是修改输出的字符串了：
```
! msg1 处放置字符串
msg1:
! 换行 + 回车
    .byte   13,10
    .ascii  "Hello OS world, my name is LZJ"
! 两对换行 + 回车
    .byte   13,10,13,10

! boot_flag 必须在最后两个字节
.org 510
! 设置引导扇区标记 0xAA55
! 必须有它，才能引导
boot_flag:
    .word   0xAA55
copy
```
将 .org 508 修改为 .org 510，是因为这里不需要 root_dev: .word ROOT_DEV，为了保证 boot_flag 一定在最后两个字节，所以要修改 .org。

完整的代码如下：
```
entry _start
_start:
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov cx,#36
    mov bx,#0x0007
    mov bp,#msg1
    mov ax,#0x07c0
    mov es,ax
    mov ax,#0x1301
    int 0x10
inf_loop:
    jmp inf_loop
msg1:
    .byte   13,10
    .ascii  "Hello OS world, my name is LZJ"
    .byte   13,10,13,10
.org 510
boot_flag:
    .word   0xAA55
copy
```
接下来，将完成屏幕显示的代码在开发环境中编译，并将编译后的目标文件做成 Image 文件。

## 编译和运行
Ubuntu 上先从终端进入 ~/oslab/linux-0.11/boot/ 目录。

Windows 上则先双击快捷方式 “MinGW32.bat”，将打开一个命令行窗口，当前目录是 oslab，用 cd 命令进入 linux-0.11\boot。

无论那种系统，都执行下面两个命令编译和链接 bootsect.s：

$ as86 -0 -a -o bootsect.o bootsect.s
$ ld86 -0 -s -o bootsect bootsect.o
copy
其中 -0（注意：这是数字 0，不是字母 O）表示生成 8086 的 16 位目标程序，-a 表示生成与 GNU as 和 ld 部分兼容的代码，-s 告诉链接器 ld86 去除最后生成的可执行文件中的符号信息。

如果这两个命令没有任何输出，说明编译与链接都通过了。

Ubuntu 下用 ls -l 可列出下面的信息：

-rw--x--x    1  root  root  544  Jul  25  15:07   bootsect
-rw------    1  root  root  257  Jul  25  15:07   bootsect.o
-rw------    1  root  root  686  Jul  25  14:28   bootsect.s
copy
Windows 下用 dir 可列出下面的信息：

2008-07-28  20:14               544 bootsect
2008-07-28  20:14               924 bootsect.o
2008-07-26  20:13             5,059 bootsect.s
copy
其中 bootsect.o 是中间文件。bootsect 是编译、链接后的目标文件。

需要留意的文件是 bootsect 的文件大小是 544 字节，而引导程序必须要正好占用一个磁盘扇区，即 512 个字节。造成多了 32 个字节的原因是 ld86 产生的是 Minix 可执行文件格式，这样的可执行文件除了文本段、数据段等部分以外，还包括一个 Minix 可执行文件头部，它的结构如下：

struct exec {
    unsigned char a_magic[2];  //执行文件魔数
    unsigned char a_flags;
    unsigned char a_cpu;       //CPU标识号
    unsigned char a_hdrlen;    //头部长度，32字节或48字节
    unsigned char a_unused;
    unsigned short a_version;
    long a_text; long a_data; long a_bss; //代码段长度、数据段长度、堆长度
    long a_entry;    //执行入口地址
    long a_total;    //分配的内存总量
    long a_syms;     //符号表大小
};
copy
算一算：6 char（6 字节）+ 1 short（2 字节） + 6 long（24 字节）= 32，正好是 32 个字节，去掉这 32 个字节后就可以放入引导扇区了（这是 tools/build.c 的用途之一）。

对于上面的 Minix 可执行文件，其 a_magic[0]=0x01，a_magic[1]=0x03，a_flags=0x10（可执行文件），a_cpu=0x04（表示 Intel i8086/8088，如果是 0x17 则表示 Sun 公司的 SPARC），所以 bootsect 文件的头几个字节应该是 01 03 10 04。为了验证一下，Ubuntu 下用命令“hexdump -C bootsect”可以看到：

00000000  01 03 10 04 20 00 00 00  00 02 00 00 00 00 00 00  |.... ...........|
00000010  00 00 00 00 00 00 00 00  00 82 00 00 00 00 00 00  |................|
00000020  b8 c0 07 8e d8 8e c0 b4  03 30 ff cd 10 b9 17 00  |.........0......|
00000030  bb 07 00 bd 3f 00 b8 01  13 cd 10 b8 00 90 8e c0  |....?...........|
00000040  ba 00 00 b9 02 00 bb 00  02 b8 04 02 cd 13 73 0a  |..............s.|
00000050  ba 00 00 b8 00 00 cd 13  eb e1 ea 00 00 20 90 0d  |............. ..|
00000060  0a 53 75 6e 69 78 20 69  73 20 72 75 6e 6e 69 6e  |.Sunix is runnin|
00000070  67 21 0d 0a 0d 0a 00 00  00 00 00 00 00 00 00 00  |g!..............|
00000080  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000210  00 00 00 00 00 00 00 00  00 00 00 00 00 00 55 aa  |..............U.|
00000220
copy
Windows 下用 UltraEdit 把该文件打开，果然如此。

img

图 1 用 UltraEdit 打开文件 bootsect
接下来干什么呢？是的，要去掉这 32 个字节的文件头部（tools/build.c 的功能之一就是这个）！随手编个小的文件读写程序都可以去掉它。不过，懒且聪明的人会在 Ubuntu 下用命令：

$ dd bs=1 if=bootsect of=Image skip=32
copy
生成的 Image 就是去掉文件头的 bootsect。

Windows 下可以用 UltraEdit 直接删除（选中这 32 个字节，然后按 Ctrl+X）。

去掉这 32 个字节后，将生成的文件拷贝到 linux-0.11 目录下，并一定要命名为“Image”（注意大小写）。然后就“run”吧！

# 当前的工作路径为 /home/shiyanlou/oslab/linux-0.11/boot/

# 将刚刚生成的 Image 复制到 linux-0.11 目录下
$ cp ./Image ../Image

# 执行 oslab 目录中的 run 脚本
$ ../../run
copy
img

图 2 bootsect 引导后的系统启动情况
