# 地址映射与共享
## 课程说明

本实验是 操作系统之内存管理 - 网易云课堂 的配套实验，推荐大家进行实验之前先学习相关课程：

- L20 内存使用与分段
- L21 内存分区与分页
- L22 段页结合的实际内存管理
- L23 请求调页内存换入
- L24 内存换出

> Tips：点击上方文字中的超链接或者输入 https://mooc.study.163.com/course/1000003007#/info 进入理论课程的学习。 如果网易云上的课程无法查看，也可以看 Bilibili 上的 操作系统哈尔滨工业大学李治军老师。

## 实验目的
- 深入理解操作系统的段、页式内存管理，深入理解段表、页表、逻辑地址、线性地址、物理地址等概念；
- 实践段、页式内存管理的地址映射过程；
- 编程实现段、页式内存管理上的内存共享，从而深入理解操作系统的内存管理。

## 实验内容
本次实验的基本内容是：

- 用 Bochs 调试工具跟踪 Linux 0.11 的地址翻译（地址映射）过程，了解 IA-32 和 Linux 0.11 的内存管理机制；
- 在 Ubuntu 上编写多进程的生产者—消费者程序，用共享内存做缓冲区；
- 在信号量实验的基础上，为 Linux 0.11 增加共享内存功能，并将生产者—消费者程序移植到 Linux 0.11。

### 跟踪地址翻译过程
首先以汇编级调试的方式启动 Bochs，引导 Linux 0.11，在 0.11 下编译和运行 test.c。它是一个无限循环的程序，永远不会主动退出。然后在调试器中通过查看各项系统参数，从逻辑地址、LDT 表、GDT 表、线性地址到页表，计算出变量 i 的物理地址。最后通过直接修改物理内存的方式让 test.c 退出运行。

test.c 的代码如下：
```c
#include <stdio.h>

int i = 0x12345678;
int main(void)
{
    printf("The logical/virtual address of i is 0x%08x", &i);
    fflush(stdout);
    while (i)
        ;
    return 0;
}
```
### 基于共享内存的生产者—消费者程序
本项实验在 Ubuntu 下完成，与信号量实验中的 pc.c 的功能要求基本一致，仅有两点不同：

- 不用文件做缓冲区，而是使用共享内存；
- 生产者和消费者分别是不同的程序。生产者是 `producer.c`，消费者是 `consumer.c`。两个程序都是单进程的，通过信号量和缓冲区进行通信。

Linux 下，可以通过 `shmget()` 和 `shmat()` 两个系统调用使用共享内存。

### 共享内存的实现
进程之间可以通过页共享进行通信，被共享的页叫做共享内存，结构如下图所示：

![share](./README.assets/share.jpg)
 
本部分实验内容是在 Linux 0.11 上实现上述页面共享，并将上一部分实现的 `producer.c` 和 `consumer.c` 移植过来，验证页面共享的有效性。

具体要求在 `mm/shm.c` 中实现 `shmget()` 和 `shmat()` 两个系统调用。它们能支持 `producer.c` 和 `consumer.c` 的运行即可，不需要完整地实现 POSIX 所规定的功能。
```c
shmget()
int shmget(key_t key, size_t size, int shmflg);
```

`shmget()` 会新建/打开一页内存，并返回该页共享内存的 `shmid`（该块共享内存在操作系统内部的 `id`）。

所有使用同一块共享内存的进程都要使用相同的 `key` 参数。

如果`key` 所对应的共享内存已经建立，则直接返回 `shmid`。如果 `size` 超过一页内存的大小，返回 `-1`，并置 `errno` 为 `EINVAL`。如果系统无空闲内存，返回 `-1`，并置 `errno` 为 `ENOMEM`。

`shmflg` 参数可忽略。
```c
shmat()
void *shmat(int shmid, const void *shmaddr, int shmflg);
```
`shmat()` 会将 shmid 指定的共享页面映射到当前进程的虚拟地址空间中，并将其首地址返回。

如果 `shmid` 非法，返回 `-1`，并置 `errno` 为 `EINVAL`。

`shmaddr` 和 `shmflg` 参数可忽略。

## 实验提示
本次需要完成的内容：

（1）用 Bochs 调试工具跟踪 Linux 0.11 的地址翻译（地址映射）过程，了解 IA-32 和 Linux 0.11 的内存管理机制；

（2）在 Ubuntu 上编写多进程的生产者—消费者程序，用共享内存做缓冲区；

（3）在信号量实验的基础上，为 Linux 0.11 增加共享内存功能，并将生产者—消费者程序移植到 Linux 0.11。

### IA-32 的地址翻译过程
Linux 0.11 完全遵循 IA-32（Intel Architecture 32-bit）架构进行地址翻译，Windows、后续版本的 Linux 以及一切在 IA-32 保护模式下运行的操作系统都遵循此架构。因为只有这样才能充分发挥 CPU 的 `MMU（内存管理单元）` 的功能。

关于此地址翻译过程的细节，请参考《注释》一书中的 5.3.1-5.3.4 节。

### 用 Bochs 汇编级调试功能进行人工地址翻译
此过程比较机械，基本不消耗脑细胞，做一下有很多好处。

（1）准备
编译好 Linux 0.11 后，首先通过运行 `./dbg-asm` 启动调试器，此时 Bochs 的窗口处于黑屏状态

![boch1](./README.assets/boch1.jpg)

而命令行窗口显示：

![boch2](./README.assets/boch2.jpg)

```shell
========================================================================
                       Bochs x86 Emulator 2.3.7
               Build from CVS snapshot, on June 3, 2008
========================================================================
00000000000i[     ] reading configuration from ./bochs/bochsrc.bxrc
00000000000i[     ] installing x module as the Bochs GUI
00000000000i[     ] using log file ./bochsout.txt
Next at t=0
(0) [0xfffffff0] f000:fff0 (unk. ctxt): jmp far f000:e05b         ; ea5be000f0
<bochs:1>_
```
`Next at t=0` 表示下面的指令是 Bochs 启动后要执行的第一条软件指令。

单步跟踪进去就能看到 BIOS 的代码。不过这不是本实验需要的。直接输入命令 `c`，continue 程序的运行，Bochs 一如既往地启动了 Linux 0.11。

在 Linux 0.11 下输入（或拷入）`test.c`（代码在本实验的第 3 小节中），编译为 `test`，运行之，打印如下信息：
```
The logical/virtual address of i is 0x00003004
```
只要 `test` 不变，`0x00003004` 这个值在任何人的机器上都是一样的。即使在同一个机器上多次运行 `test`，也是一样的。

`test` 是一个死循环，只会不停占用 CPU，不会退出。

（2）暂停
当 `test` 运行的时候，在命令行窗口按 `Ctrl+c`，Bochs 会暂停运行，进入调试状态。绝大多数情况下都会停在 `test` 内，显示类似如下信息：
```
(0) [0x00fc8031] 000f:00000031 (unk. ctxt): cmp dword ptr ds:0x3004, 0x00000000 ; 833d0430000000
```
其中的 `000f` 如果是 `0008`，则说明中断在了内核里。那么就要 `c`，然后再 `ctrl+c`，直到变为 `000f` 为止。

如果显示的下一条指令不是 `cmp ...`（这里指语句以 `cmp` 开头），就用 `n` 命令单步运行几步，直到停在 `cmp ...`。

使用命令 `u /8`，显示从当前位置开始 8 条指令的反汇编代码，结构如下：

![boch3](./README.assets/boch3.jpg)

```shell
<bochs:3> u /8
10000063: (                    ): cmp dword ptr ds:0x3004, 0x00000000 ; 833d0430000000
1000006a: (                    ): jz .+0x00000004           ; 7404
1000006c: (                    ): jmp .+0xfffffff5          ; ebf5
1000006e: (                    ): add byte ptr ds:[eax], al ; 0000
10000070: (                    ): xor eax, eax              ; 31c0
10000072: (                    ): jmp .+0x00000000          ; eb00
10000074: (                    ): leave                     ; c9
10000075: (                    ): ret                       ; c3
```
这就是 `test.c` 中从 `while` 开始一直到 `return` 的汇编代码。变量 `i` 保存在 `ds:0x3004` 这个地址，并不停地和 `0` 进行比较，直到它为 `0`，才会跳出循环。

现在，开始寻找 `ds:0x3004` 对应的物理地址。

### 段表

`ds:0x3004` 是虚拟地址，`ds` 表明这个地址属于 `ds` 段。首先要找到段表，然后通过 `ds` 的值在段表中找到 `ds` 段的具体信息，才能继续进行地址翻译。

每个在 `IA-32` 上运行的应用程序都有一个段表，叫 `LDT`，段的信息叫段描述符。

`LDT` 在哪里呢？`ldtr` 寄存器是线索的起点，通过它可以在 `GDT`（全局描述符表）中找到 `LDT` 的物理地址。

用 `sreg` 命令（是在调试窗口输入）：
```shell
<bochs:4> sreg
cs:s=0x000f, dl=0x00000002, dh=0x10c0fa00, valid=1
ds:s=0x0017, dl=0x00003fff, dh=0x10c0f300, valid=3
ss:s=0x0017, dl=0x00003fff, dh=0x10c0f300, valid=1
es:s=0x0017, dl=0x00003fff, dh=0x10c0f300, valid=1
fs:s=0x0017, dl=0x00003fff, dh=0x10c0f300, valid=1
gs:s=0x0017, dl=0x00003fff, dh=0x10c0f300, valid=1
ldtr:s=0x0068, dl=0xa2d00068, dh=0x000082fa, valid=1
tr:s=0x0060, dl=0xa2e80068, dh=0x00008bfa, valid=1
gdtr:base=0x00005cb8, limit=0x7ff
idtr:base=0x000054b8, limit=0x7ff
```
可以看到 ldtr 的值是 `0x0068=0000000001101000`（二进制），表示 `LDT` 表存放在 `GDT` 表的 `1101`（二进制）=`13`（十进制）号位置（每位数据的意义参考后文叙述的段选择子）。

而 `GDT` 的位置已经由 `gdtr` 明确给出，在物理地址的 `0x00005cb8`。

用`xp /32w 0x00005cb8` 查看从该地址开始，32 个字的内容，及 `GDT` 表的前 16 项，如下：
```shell
<bochs:5> xp /32w 0x00005cb8
[bochs]:
0x00005cb8 <bogus+       0>:    0x00000000    0x00000000    0x00000fff    0x00c09a00
0x00005cc8 <bogus+      16>:    0x00000fff    0x00c09300    0x00000000    0x00000000
0x00005cd8 <bogus+      32>:    0xa4280068    0x00008901    0xa4100068    0x00008201
0x00005ce8 <bogus+      48>:    0xf2e80068    0x000089ff    0xf2d00068    0x000082ff
0x00005cf8 <bogus+      64>:    0xd2e80068    0x000089ff    0xd2d00068    0x000082ff
0x00005d08 <bogus+      80>:    0x12e80068    0x000089fc    0x12d00068    0x000082fc
0x00005d18 <bogus+      96>:    0xa2e80068    0x00008bfa    0xa2d00068    0x000082fa
0x00005d28 <bogus+     112>:    0xc2e80068    0x000089f8    0xc2d00068    0x000082f8
```
GDT 表中的每一项占 64 位（8 个字节），所以我们要查找的项的地址是 `0x00005cb8+13*8`。

输入 `xp /2w 0x00005cb8+13*8`，得到：
```shell
<bochs:6> xp /2w 0x00005cb8+13*8
[bochs]:
0x00005d20 <bogus+       0>:    0xa2d00068    0x000082fa
```
上两步看到的数值可能和这里给出的示例不一致，这是很正常的。如果想确认是否准确，就看 `sreg` 输出中，`ldtr` 所在行里，`dl` 和 `dh` 的值，它们是 Bochs 的调试器自动计算出的，你寻找到的必须和它们一致。

`“0xa2d00068 0x000082fa”` 将其中的加粗数字组合为“0x00faa2d0”，这就是 `LDT` 表的物理地址（为什么这么组合，参考后文介绍的段描述符）。

`xp /8w 0x00faa2d0`，得到：
```shell
<bochs:7> xp /8w 0x00faa2d0
[bochs]:
0x00faa2d0 <bogus+       0>:    0x00000000    0x00000000    0x00000002    0x10c0fa00
0x00faa2e0 <bogus+      16>:    0x00003fff    0x10c0f300    0x00000000    0x00fab000
```
这就是 LDT 表的前 4 项内容了。

### 段描述符
在保护模式下，**段寄存器**有另一个名字，叫**段选择子**，因为它保存的信息主要是该段在段表里索引值，用这个索引值可以从段表中“选择”出相应的段描述符。

先看看 `ds` 选择子的内容，还是用 `sreg` 命令：

```shell
<bochs:8> sreg
cs:s=0x000f, dl=0x00000002, dh=0x10c0fa00, valid=1
ds:s=0x0017, dl=0x00003fff, dh=0x10c0f300, valid=3
ss:s=0x0017, dl=0x00003fff, dh=0x10c0f300, valid=1
es:s=0x0017, dl=0x00003fff, dh=0x10c0f300, valid=1
fs:s=0x0017, dl=0x00003fff, dh=0x10c0f300, valid=1
gs:s=0x0017, dl=0x00003fff, dh=0x10c0f300, valid=1
ldtr:s=0x0068, dl=0xa2d00068, dh=0x000082fa, valid=1
tr:s=0x0060, dl=0xa2e80068, dh=0x00008bfa, valid=1
gdtr:base=0x00005cb8, limit=0x7ff
idtr:base=0x000054b8, limit=0x7ff
```

可以看到，`ds` 的值是 `0x0017`。段选择子是一个 16 位寄存器，它各位的含义如下图：

![seg_index](./README.assets/seg_index.jpg)

其中 RPL 是请求特权级，当访问一个段时，处理器要检查 RPL 和 CPL（放在 cs 的位 0 和位 1 中，用来表示当前代码的特权级），即使程序有足够的特权级（CPL）来访问一个段，但如果 RPL（如放在 ds 中，表示请求数据段）的特权级不足，则仍然不能访问，即如果 RPL 的数值大于 CPL（数值越大，权限越小），则用 RPL 的值覆盖 CPL 的值。

而段选择子中的 TI 是表指示标记，如果 TI=0，则表示段描述符（段的详细信息）在 GDT（全局描述符表）中，即去 GDT 中去查；而 TI=1，则去 LDT（局部描述符表）中去查。

看看上面的 `ds`，`0x0017=0000000000010111`（二进制），所以 RPL=11，可见是在最低的特权级（因为在应用程序中执行），TI=1，表示查找 LDT 表，索引值为 10（二进制）= 2（十进制），表示找 LDT 表中的第 3 个段描述符（从 0 开始编号）。

LDT 和 GDT 的结构一样，每项占 8 个字节。所以第 3 项 `0x00003fff 0x10c0f300`（上一步骤的最后一个输出结果中） 就是搜寻好久的 ds 的段描述符了。

用 `sreg` 输出中 `ds` 所在行的 `dl` 和 `dh` 值可以验证找到的描述符是否正确。

接下来看看段描述符里面放置的是什么内容：

![seg_desc](./README.assets/seg_desc.jpg)

可以看到，段描述符是一个 64 位二进制的数，存放了段基址和段限长等重要的数据。其中位 P（Present）是段是否存在的标记；位 S 用来表示是系统段描述符（S=0）还是代码或数据段描述符（S=1）；四位 TYPE 用来表示段的类型，如数据段、代码段、可读、可写等；DPL 是段的权限，和 CPL、RPL 对应使用；位 G 是粒度，G=0 表示段限长以位为单位，G=1 表示段限长以 4KB 为单位；其他内容就不详细解释了。

### 段基址和线性地址
费了很大的劲，实际上我们需要的只有段基址一项数据，即段描述符 `“0x00003fff 0x10c0f300”` 中加粗部分组合成的` “0x10000000”`。这就是 ds 段在线性地址空间中的起始地址。用同样的方法也可以算算其它段的基址，都是这个数。

段基址+段内偏移，就是线性地址了。所以 `ds:0x3004` 的线性地址就是：
```
0x10000000 + 0x3004 = 0x10003004
```
用 `calc ds:0x3004` 命令可以验证这个结果。

### 页表
从线性地址计算物理地址，需要查找页表。线性地址变成物理地址的过程如下：

![page](./README.assets/page.jpg)

线性地址变成物理地址

首先需要算出线性地址中的页目录号、页表号和页内偏移，它们分别对应了 32 位线性地址的 10 位 + 10 位 + 12 位，所以 `0x10003004` 的页目录号是 `64`，页号 `3`，页内偏移是`4`。

IA-32 下，页目录表的位置由`CR3` 寄存器指引。`“creg”`命令可以看到：
``` shell
CR0=0x8000001b: PG cd nw ac wp ne ET TS em MP PE
CR2=page fault laddr=0x10002f68
CR3=0x00000000
    PCD=page-level cache disable=0
    PWT=page-level writes transparent=0
CR4=0x00000000: osxmmexcpt osfxsr pce pge mce pae pse de tsd pvi vme
```
说明页目录表的基址为 0。看看其内容，```xp /68w 0```：
```shell
0x00000000 :    0x00001027    0x00002007    0x00003007    0x00004027
0x00000010 :    0x00000000    0x00024764    0x00000000    0x00000000
0x00000020 :    0x00000000    0x00000000    0x00000000    0x00000000
0x00000030 :    0x00000000    0x00000000    0x00000000    0x00000000
0x00000040 :    0x00ffe027    0x00000000    0x00000000    0x00000000
0x00000050 :    0x00000000    0x00000000    0x00000000    0x00000000
0x00000060 :    0x00000000    0x00000000    0x00000000    0x00000000
0x00000070 :    0x00000000    0x00000000    0x00000000    0x00000000
0x00000080 :    0x00ff3027    0x00000000    0x00000000    0x00000000
0x00000090 :    0x00000000    0x00000000    0x00000000    0x00000000
0x000000a0 :    0x00000000    0x00000000    0x00000000    0x00000000
0x000000b0 :    0x00000000    0x00000000    0x00000000    0x00ffb027
0x000000c0 :    0x00ff6027    0x00000000    0x00000000    0x00000000
0x000000d0 :    0x00000000    0x00000000    0x00000000    0x00000000
0x000000e0 :    0x00000000    0x00000000    0x00000000    0x00000000
0x000000f0 :    0x00000000    0x00000000    0x00000000    0x00ffa027
0x00000100 :    0x00faa027    0x00000000    0x00000000    0x00000000
```
页目录表和页表中的内容很简单，是 1024 个 32 位（正好是 4K）数。这 32 位中前 20 位是物理页框号，后面是一些属性信息（其中最重要的是最后一位 P）。其中第 65 个页目录项就是我们要找的内容，用`“xp /w 0+64*4”`查看：
```shell
0x00000100 :    0x00faa027
```
其中的 `027` 是属性，显然 `P=1`，其他属性实验者自己分析吧。页表所在物理页框号为 `0x00faa`，即页表在物理内存的 `0x00faa000` 位置。从该位置开始查找 3 号页表项，得到```（xp /w 0x00faa000+3*4）```：
```
0x00faa00c :    0x00fa7067
```
其中 `067` 是属性，显然 `P=1`，应该是这样。

### 物理地址
最终结果马上就要出现了！

线性地址 `0x10003004` 对应的物理页框号为 `0x00fa7`，和页内偏移 `0x004` 接到一起，得到 `0x00fa7004`，这就是变量 `i` 的物理地址。可以通过两种方法验证。

第一种方法是用命令 `page 0x10003004`，可以得到信息：
```shell
linear page 0x10003000 maps to physical page 0x00fa7000
```
第二种方法是用命令 `xp /w 0x00fa7004`，可以看到：
```shell
0x00fa7004 :    0x12345678
```
这个数值确实是 `test.c` 中 `i` 的初值。

现在，通过直接修改内存来改变 `i` 的值为 `0`，命令是： `setpmem 0x00fa7004 4 0`，表示从 `0x00fa7004` 地址开始的 4 个字节都设为 0。然后再用`c`命令继续 Bochs 的运行，可以看到 `test` 退出了，说明 `i` 的修改成功了，此项实验结束。

### 在 Linux 0.11 中实现共享内存

#### （1）Linux 中的共享内存
Linux 支持两种方式的共享内存。一种方式是 `shm_open()`、`mmap()` 和 `shm_unlink()` 的组合；另一种方式是 `shmget()`、`shmat()` 和 `shmdt()` 的组合。本实验建议使用后一种方式。

这些系统调用的详情，请查阅 `man` 及相关资料。

特别提醒：没有父子关系的进程之间进行共享内存，`shmget()` 的第一个参数`key` 不要用 `IPC_PRIVATE`，否则无法共享。用什么数字可视心情而定。

#### （2）获得空闲物理页面
实验者需要考虑如何实现页面共享。首先看一下 Linux 0.11 如何操作页面，如何管理进程地址空间。

在 kernel/fork.c 文件中有：
```c
int copy_process(…)
{
    struct task_struct *p;
    p = (struct task_struct *) get_free_page();
    if (!p)
        return -EAGAIN;
//    ……
}
```
函数 get_free_page() 用来获得一个空闲物理页面，在 mm/memory.c 文件中：
```c
unsigned long get_free_page(void)
{
    register unsigned long __res asm("ax");
    __asm__("std ; repne ; scasb\n\t"
            "jne 1f\n\t"
            "movb $1,1(%%edi)\n\t"
            // 页面数*4KB=相对页面起始地址
            "sall $12,%%ecx\n\t"
            // 在加上低端的内存地址，得到的是物理起始地址
            "addl %2,%%ecx\n\t"
            "movl %%ecx,%%edx\n\t"
            "movl $1024,%%ecx\n\t"
            "leal 4092(%%edx),%%edi\n\t"
            "rep ; stosl\n\t"
            //edx赋给eax，eax返回了物理起始地址
            "movl %%edx,%%eax\n"
            "1:" :"=a" (__res) :"0" (0),"i" (LOW_MEM),"c" (PAGING_PAGES),
            "D" (mem_map+PAGING_PAGES-1):"di","cx","dx");
    return __res;
}

static unsigned char mem_map [ PAGING_PAGES ] = {0,};
```
显然 `get_free_page` 函数就是在 `mem_map` 位图中寻找值为 0 的项（空闲页面），该函数返回的是该页面的起始物理地址。

#### （3）地址映射
有了空闲的物理页面，接下来需要完成线性地址和物理页面的映射，Linux 0.11 中也有这样的代码，看看 `mm/memory.c` 中的 `do_no_page(unsigned long address)`，该函数用来处理线性地址`address` 对应的物理页面无效的情况（即缺页中断），`do_no_page` 函数中调用一个重要的函数 `get_empty_page(address)`，其中有：
```c
// 函数 get_empty_page(address)

unsigned long tmp=get_free_page();
// 建立线性地址和物理地址的映射
put_page(tmp, address);
```

显然这两条语句就用来获得空闲物理页面，然后填写线性地址 address 对应的页目录和页表。

#### （4）寻找空闲的虚拟地址空间
有了空闲物理页面，也有了建立线性地址和物理页面的映射，但要完成本实验还需要能获得一段空闲的虚拟地址空闲。

要从数据段中划出一段空间，首**先需要了解进程数据段空间的分布，而这个分布显然是由 `exec` 系统调用决定的**，所以要详细看一看 `exec` 的核心代码，`do_execve`（在文件 `fs/exec.c` 中）。

在函数 `do_execve()` 中，修改数据段（当然是修改 LDT）的地方是 `change_ldt`，函数 `change_ldt` 实现如下：
```c
static unsigned long change_ldt(unsigned long text_size,unsigned long * page)
{
    /*其中text_size是代码段长度，从可执行文件的头部取出，page为参数和环境页*/
    unsigned long code_limit,data_limit,code_base,data_base;
    int i;

    code_limit = text_size+PAGE_SIZE -1;
    code_limit &= 0xFFFFF000;
    //code_limit为代码段限长=text_size对应的页数（向上取整）
    data_limit = 0x4000000; //数据段限长64MB
    code_base = get_base(current->ldt[1]);
    data_base = code_base;

    // 数据段基址 = 代码段基址
    set_base(current->ldt[1],code_base);
    set_limit(current->ldt[1],code_limit);
    set_base(current->ldt[2],data_base);
    set_limit(current->ldt[2],data_limit);
    __asm__("pushl $0x17\n\tpop %%fs":: );

    // 从数据段的末尾开始
    data_base += data_limit;

    // 向前处理
    for (i=MAX_ARG_PAGES-1 ; i>=0 ; i--) {
        // 一次处理一页
        data_base -= PAGE_SIZE;
        // 建立线性地址到物理页的映射
        if (page[i]) put_page(page[i],data_base);
    }
    // 返回段界限
    return data_limit;
}
```
仔细分析过函数 `change_ldt`，想必实验者已经知道该如何从数据段中找到一页空闲的线性地址。《注释》中的图 13-6 也能给你很大帮助。

### 在同一终端中同时运行两个程序
Linux 的 shell 有后台运行程序的功能。只要在命令的最后输入一个 `&`，命令就会进入后台运行，前台马上回到提示符，进而能运行下一个命令，例如：
```shell
$ sudo ./producer &
$ sudo ./consumer
```
当运行 `./consumer` 的时候，`producer` 正在后台运行。

