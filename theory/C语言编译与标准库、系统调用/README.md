## 标准库与系统调用
#### 系统调用

系统调用是操作系统提供，在操作系统启动过程中将系统调用函数注册在了`int 0x80`的中断位置处，因此用户程序可以通过`int 0x80`来触发系统调用，并通过`eax`、`ebx`等寄存器来传递`系统调用号`和一些`必要的系统调用参数` 来进行系统调用。 想要了解其细节可以查看文档：[中断与系统调用](https://github.com/lcdzhao/operating_system/tree/master/theory/%E4%B8%AD%E6%96%AD%E4%B8%8E%E7%B3%BB%E7%BB%9F%E8%B0%83%E7%94%A8)。

#### 标准库
标准库分为下面两部分：

- 标准库头文件：`stdio.h`等标准库头文件。
- 标准库实现：标准库的具体实现分为：
    - 静态链接库：在编译器的链接阶段注入到用户程序中。
    - 动态链接库：在应用运行阶段链接到动态链接库中。

之所以将`标准库头文件`与`标准库实现`分离开来，是为了更强的灵活性(同样的头文件与用户代码，在不同的系统中可以编译成符合当前系统的二进制程序)，符合设计模式六大原则里的`依赖倒置(面向接口编程)`(因为头文件里的函数声明就相当于JAVA中的接口)。

#### 系统调用与标准库 的关系

`系统调用`为`操作系统`提供，而`标准库的实现`由`C语言标准`提供，在`标准库的实现` 中调用了系统调用来利用操作系统资源。

采用这样的设计，符合设计模式六大设计原则里的`单一职责`：`操作系统`仅提供`系统调用`，而不需要关心外部库函数。C语言标准提供`库函数`，通过`库函数`调用`系统调用`，而不需要关注`系统调用`具体如何实现。

关于`系统调用与标准库`具体是如何连接起来的详细分析，可以查看当前文章的**printf 背后的故事**的这一部分，这一部分详细分析了`printf`是如何被编译成二进制机器码，又是如何调用到`系统调用`。如果对于C语言编译不熟悉的话，可以先看一下下面的`C语言编译`部分。

## C 语言编译

> 该部分内容转载自：[C/C++程序编译过程详解](https://www.cnblogs.com/mickole/articles/3659112.html) <br>
> 作者：mickole <br>
> 出处：http://www.cnblogs.com/mickole/

C语言的编译链接过程要把我们编写的一个c程序（源代码）转换成可以在硬件上运行的程序（可执行代码），需要进行编译和链接。编译就是把文本形式源代码翻译为机器语言形式的目标文件的过程。链接是把目标文件、操作系统的启动代码和用到的库文件进行组织，形成最终生成可执行代码的过程。过程图解如下：

clip_image002

从图上可以看到，整个代码的编译过程分为编译和链接两个过程，编译对应图中的大括号括起的部分，其余则为链接过程。

1. 编译过程
编译过程又可以分成两个阶段：编译和汇编。

编译
编译是读取源程序（字符流），对之进行词法和语法的分析，将高级语言指令转换为功能等效的汇编代码，源文件的编译过程包含两个主要阶段：

编译预处理
读取c源程序，对其中的伪指令（以# 开头的指令）和特殊符号进行处理。

伪指令主要包括以下四个方面：

1) 宏定义指令，如# define Name TokenString，# undef等。

对于前一个伪指令，预编译所要做的是将程序中的所有Name用TokenString替换，但作为字符串常量的 Name则不被替换。对于后者，则将取消对某个宏的定义，使以后该串的出现不再被替换。

2) 条件编译指令，如# ifdef，# ifndef，# else，# elif，# endif等。

这些伪指令的引入使得程序员可以通过定义不同的宏来决定编译程序对哪些代码进行处理。预编译程序将根据有关的文件，将那些不必要的代码过滤掉。

3) 头文件包含指令，如# include "FileName" 或者# include < FileName> 等。

在头文件中一般用伪指令# define定义了大量的宏（最常见的是字符常量），同时包含有各种外部符号的声明。

采用头文件的目的主要是为了使某些定义可以供多个不同的C源程序使用。因为在需要用到这些定义的C源程序中，只需加上一条# include语句即可，而不必再在此文件中将这些定义重复一遍。预编译程序将把头文件中的定义统统都加入到它所产生的输出文件中，以供编译程序对之进行处理。

包含到c源程序中的头文件可以是系统提供的，这些头文件一般被放在/ usr/ include目录下。在程序中# include它们要使用尖括号（< >）。另外开发人员也可以定义自己的头文件，这些文件一般与c源程序放在同一目录下，此时在# include中要用双引号（""）。

4) 特殊符号，预编译程序可以识别一些特殊的符号。

例如在源程序中出现的LINE标识将被解释为当前行号（十进制数），FILE则被解释为当前被编译的C源程序的名称。预编译程序对于在源程序中出现的这些串将用合适的值进行替换。

预编译程序所完成的基本上是对源程序的“替代”工作。经过此种替代，生成一个没有宏定义、没有条件编译指令、没有特殊符号的输出文件。这个文件的含义同没有经过预处理的源文件是相同的，但内容有所不同。下一步，此输出文件将作为编译程序的输入而被翻译成为机器指令。

编译、优化阶段
经过预编译得到的输出文件中，只有常量；如数字、字符串、变量的定义，以及C语言的关键字，如main, if , else , for , while , { , } , + , - , * , \ 等等。

编译程序所要作得工作就是通过词法分析和语法分析，在确认所有的指令都符合语法规则之后，将其翻译成等价的中间代码表示或汇编代码。

优化处理是编译系统中一项比较艰深的技术。它涉及到的问题不仅同编译技术本身有关，而且同机器的硬件环境也有很大的关系。优化一部分是对中间代码的优化。这种优化不依赖于具体的计算机。另一种优化则主要针对目标代码的生成而进行的。

对于前一种优化，主要的工作是删除公共表达式、循环优化（代码外提、强度削弱、变换循环控制条件、已知量的合并等）、复写传播，以及无用赋值的删除，等等。

后一种类型的优化同机器的硬件结构密切相关，最主要的是考虑是如何充分利用机器的各个硬件寄存器存放有关变量的值，以减少对于内存的访问次数。另外，如何根据机器硬件执行指令的特点（如流水线、RISC、CISC、VLIW等）而对指令进行一些调整使目标代码比较短，执行的效率比较高，也是一个重要的研究课题。

经过优化得到的汇编代码必须经过汇编程序的汇编转换成相应的机器指令，方可能被机器执行。

汇编
汇编过程实际上指把汇编语言代码翻译成目标机器指令的过程。对于被翻译系统处理的每一个C语言源程序，都将最终经过这一处理而得到相应的目标文件。目标文件中所存放的也就是与源程序等效的目标的机器语言代码。

目标文件由段组成。通常一个目标文件中至少有两个段：

1) 代码段：该段中所包含的主要是程序的指令。该段一般是可读和可执行的，但一般却不可写。

2) 数据段：主要存放程序中要用到的各种全局变量或静态的数据。一般数据段都是可读，可写，可执行的。

UNIX环境下主要有三种类型的目标文件：

1) 可重定位文件

其中包含有适合于其它目标文件链接来创建一个可执行的或者共享的目标文件的代码和数据。

2) 共享的目标文件

这种文件存放了适合于在两种上下文里链接的代码和数据。

第一种是链接程序可把它与其它可重定位文件及共享的目标文件一起处理来创建另一个目标文件；

第二种是动态链接程序将它与另一个可执行文件及其它的共享目标文件结合到一起，创建一个进程映象。

3) 可执行文件

它包含了一个可以被操作系统创建一个进程来执行之的文件。

汇编程序生成的实际上是第一种类型的目标文件。对于后两种还需要其他的一些处理方能得到，这个就是链接程序的工作了。

2. 链接过程
由汇编程序生成的目标文件并不能立即就被执行，其中可能还有许多没有解决的问题。

例如，某个源文件中的函数可能引用了另一个源文件中定义的某个符号（如变量或者函数调用等）；在程序中可能调用了某个库文件中的函数，等等。所有的这些问题，都需要经链接程序的处理方能得以解决。

链接程序的主要工作就是将有关的目标文件彼此相连接，也即将在一个文件中引用的符号同该符号在另外一个文件中的定义连接起来，使得所有的这些目标文件成为一个能够被操作系统装入执行的统一整体。

根据开发人员指定的同库函数的链接方式的不同，链接处理可分为两种：

1) 静态链接

在这种链接方式下，函数的代码将从其所在的静态链接库中被拷贝到最终的可执行程序中。这样该程序在被执行时这些代码将被装入到该进程的虚拟地址空间中。静态链接库实际上是一个目标文件的集合，其中的每个文件含有库中的一个或者一组相关函数的代码。

2) 动态链接

在此种方式下，函数的代码被放到称作是动态链接库或共享对象的某个目标文件中。链接程序此时所作的只是在最终的可执行程序中记录下共享对象的名字以及其它少量的登记信息。在此可执行文件被执行时，动态链接库的全部内容将被映射到运行时相应进程的虚地址空间。动态链接程序将根据可执行程序中记录的信息找到相应的函数代码。

对于可执行文件中的函数调用，可分别采用动态链接或静态链接的方法。使用动态链接能够使最终的可执行文件比较短小，并且当共享对象被多个进程使用时能节约一些内存，因为在内存中只需要保存一份此共享对象的代码。但并不是使用动态链接就一定比使用静态链接要优越。在某些情况下动态链接可能带来一些性能上损害。

3. GCC的编译链接
我们在linux使用的gcc编译器便是把以上的几个过程进行捆绑，使用户只使用一次命令就把编译工作完成，这的确方便了编译工作，但对于初学者了解编译过程就很不利了，下图便是gcc代理的编译过程：

clip_image004

从上图可以看到：

1) 预编译

将.c 文件转化成 .i文件

使用的gcc命令是：gcc –E 文件名 -o 处理后的文件名

对应于预处理命令cpp

2) 编译

将.c/.h文件转换成.s文件

使用的gcc命令是：gcc –S 文件名 -o 处理后的文件名

对应于编译命令 cc –S

3) 汇编

将.s 文件转化成 .o文件

使用的gcc 命令是：gcc –c 文件名 -o 处理后的文件名

对应于汇编命令是 as

4) 链接

将.o文件转化成可执行程序

使用的gcc 命令是： gcc 文件名 -o 处理后的文件名

对应于链接命令是 ld

总结起来编译过程就上面的四个过程：预编译处理(.c) －－> 编译、优化程序（.s、.asm）－－> 汇编程序(.obj、.o、.a、.ko) －－> 链接程序（.exe、.elf、.axf等）。
可以通过`gcc -v test.c 2>&1`来查看编译器的参数。

4. 总结
C语言编译的整个过程是非常复杂的，里面涉及到的编译器知识、硬件知识、工具链知识都是非常多的，深入了解整个编译过程对工程师理解应用程序的编写是有很大帮助的，希望大家可以多了解一些，在遇到问题时多思考、多实践。

一般情况下，我们只需要知道分成编译和链接两个阶段，编译阶段将源程序（*.c) 转换成为目标代码（一般是obj文件，至于具体过程就是上面说的那些阶段），链接阶段是把源程序转换成的目标代码（obj文件）与你程序里面调用的库函数对应的代码连接起来形成对应的可执行文件（exe文件）就可以了，其他的都需要在实践中多多体会才能有更深的理解。



## printf 背后的故事

> 本部分内容转载自：[printf背后的故事](https://www.cnblogs.com/fanzhidongyzby/p/3519838.html) <br>
> 作者：Florian <br>
> 本文版权归作者和博客园共有，欢迎转载，但未经作者同意必须保留此段声明，且在文章页面明显位置给出原文链接，否则作者保留追究法律责任的权利。

说起编程语言，C语言大家再熟悉不过。说起最简单的代码，Helloworld更是众所周知。一条简单的printf语句便可以完成这个简单的功能，可是printf背后到底做了什么事情呢？可能很多人不曾在意，也或许你比我还要好奇！那我们就聊聊printf背后的故事。

一、printf的代码在哪里？

显然，Helloworld的源代码需要经过编译器编译，操作系统的加载才能正确执行。而编译器包含预编译、编译、汇编和链接四个步骤。

#include<stdio.h>

int main()

{

    printf("Hello World !\n");

    return 0;

}

首先，预编译器处理源代码中的宏，比如#include。预编译结束后，我们发现printf函数的声明。

$/usr/lib/gcc/i686-linux-gnu/4.7/cc1 -E -quiet              \

    main.c -o main.i

# 1 "main.c"

# 1 "<命令行>"

# 1 "main.c"

...

extern int printf (const char *__restrict __format, ...);

...

int main()

{

 printf("Hello World！\n");

 return 0;

}

然后编译器将高级语言程序转化为汇编代码。

$/usr/lib/gcc/i686-linux-gnu/4.7/cc1 -fpreprocessed -quiet  \

    main.i -o main.s

    .file      "main.c"

    .section   .rodata

.LC0:

    .string    "Hello World!"

    .text

    .globl     main

    .type      main, @function

main:

    pushl      %ebp

    movl       %esp,  %ebp

    andl       $-16,  %esp

    subl       $16,   %esp

    movl       $.LC0, (%esp)

    call       puts

    movl       $0,    %eax

    leave

    ret

    .size      main, .-main

...

我们发现printf函数调用被转化为call puts指令，而不是call printf指令，这好像有点出乎意料。不过不用担心，这是编译器对printf的一种优化。实践证明，对于printf的参数如果是以'\n'结束的纯字符串，printf会被优化为puts函数，而字符串的结尾'\n'符号被消除。除此之外，都会正常生成call printf指令。

如果我们仍希望通过printf调用"Hello World !\n"的话，只需要按照如下方式修改即可。不过这样做就不能在printf调用结束后立即看到打印字符串了，因为puts函数可以立即刷新输出缓冲区。我们仍然使用puts作为例子继续阐述。

    .section   .rodata

.LC0:

    .string    "hello world!\n"

    ...

    call       printf

...

接下来，汇编器开始工作。将汇编文件转化为我们不能直接阅读的二进制格式——可重定位目标文件，这里我们需要gcc工具包的objdump命令查看它的二进制信息。可是我们发现call puts指令里保存了无效的符号地址。

$as -o main.o main.s

$objdump –d main.o

main.o：     文件格式 elf32-i386

Disassembly of section .text:

00000000 <main>:

   0:  55                     push   %ebp

   1:  89 e5                  mov    %esp,%ebp

   3:  83 e4 f0               and    $0xfffffff0,%esp

   6:  83 ec 10               sub    $0x10,%esp

   9:  c7 04 24 00 00 00 00   movl   $0x0,(%esp)

  10:  e8 fc ff ff ff         call   11 <main+0x11>

  15:  b8 00 00 00 00         mov    $0x0,%eax

  1a:  c9                     leave 

  1b:  c3                     ret

而链接器最终会将puts的符号地址修正。由于链接方式分为静态链接和动态链接两种，虽然链接方式不同，但是不影响最终代码对库函数的调用。我们这里关注printf函数背后的原理，因此使用更易说明问题的静态链接的方式阐述。

$/usr/lib/gcc/i686-linux-gnu/4.7/collect2                   \

    -static -o main                                         \

    /usr/lib/i386-linux-gnu/crt1.o                          \

    /usr/lib/i386-linux-gnu/crti.o                          \

    /usr/lib/gcc/i686-linux-gnu/4.7/crtbeginT.o             \

    main.o                                                  \

    --start-group                                           \

    /usr/lib/gcc/i686-linux-gnu/4.7/libgcc.a                \

    /usr/lib/gcc/i686-linux-gnu/4.7/libgcc_eh.a             \

    /usr/lib/i386-linux-gnu/libc.a                          \

    --end-group                                             \

    /usr/lib/gcc/i686-linux-gnu/4.7/crtend.o                \

    /usr/lib/i386-linux-gnu/crtn.o

$objdump –sd main

Disassembly of section .text:

...

08048ea4 <main>:

 8048ea4:  55                     push   %ebp

 8048ea5:  89 e5                  mov    %esp,%ebp

 8048ea7:  83 e4 f0               and    $0xfffffff0,%esp

 8048eaa:  83 ec 10               sub    $0x10,%esp

 8048ead:  c7 04 24 e8 86 0c 08   movl   $0x80c86e8,(%esp)

 8048eb4:  e8 57 0a 00 00         call   8049910 <_IO_puts>

 8048eb9:  b8 00 00 00 00         mov    $0x0,%eax

 8048ebe:  c9                     leave 

 8048ebf:  c3                     ret

...

静态链接时，链接器将C语言的运行库（CRT）链接到可执行文件，其中crt1.o、crti.o、crtbeginT.o、crtend.o、crtn.o便是这五个核心的文件，它们按照上述命令显示的顺序分居在用户目标文件和库文件的两侧。由于我们使用了库函数puts，因此需要库文件libc.a，而libc.a与libgcc.a和libgcc_eh.a有相互依赖关系，因此需要使用-start-group和-end-group将它们包含起来。

链接后，call puts的地址被修正，但是反汇编显示的符号是_IO_puts而不是puts！难道我们找的文件不对吗？当然不是，我们使用readelf命令查看一下main的符号表。竟然发现puts和_IO_puts这两个符号的性质是等价的！objdump命令只是显示了全局的符号_IO_puts而已。

$readelf main –s

Symbol table '.symtab' contains 2307 entries:

   Num:    Value  Size Type    Bind   Vis      Ndx Name

...

  1345: 08049910   352 FUNC    WEAK   DEFAULT    6 puts

...

  1674: 08049910   352 FUNC    GLOBAL DEFAULT    6 _IO_puts

...

那么puts函数的定义真的是在libc.a里吗？我们需要对此确认。我们将libc.a解压缩，然后全局符号_IO_puts所在的二进制文件，输出结果为ioputs.o。然后查看该文件的符号表。发现ioputs.o定义了puts和_IO_puts符号，因此可以确定ioputs.o就是puts函数的代码文件，且在库文件libc.a内。

$ar -x /usr/lib/i386-linux-gnu/libc.a

$grep -rin "_IO_puts" *.o

    $readelf -s ioputs.o

Symbol table '.symtab' contains 20 entries:

   Num:    Value  Size Type    Bind   Vis      Ndx Name

...

    11: 00000000   352 FUNC    GLOBAL DEFAULT    1 _IO_puts

...

    19: 00000000   352 FUNC    WEAK   DEFAULT    1 puts

二、printf的调用轨迹

我们知道对于"Hello World !\n"的printf调用被转化为puts函数，并且我们找到了puts的实现代码是在库文件libc.a内的，并且知道它是以二进制的形式存储在文件ioputs.o内的，那么我们如何寻找printf函数的调用轨迹呢？换句话说，printf函数是如何一步步执行，最终使用Linux的int 0x80软中断进行系统调用陷入内核的呢？

如果让我们向终端输出一段字符串信息，我们一般会使用系统调用write()。那么打印Helloworld的printf最终是这样做的吗？我们借助于gdb来追踪这个过程，不过我们需要在编译源文件的时候添加-g选项，支持调试时使用符号表。

$/usr/lib/gcc/i686-linux-gnu/4.7/cc1 -fpreprocessed -quiet -g\

    main.i -o main.s

然后使用gdb调试可执行文件。

$gdb ./main

(gdb)break main

(gdb)run

(gdb)stepi

在main函数内下断点，然后调试执行，接着不断的使用stepi指令执行代码，直到看到Hello World !输出为止。这也是为什么我们使用puts作为示例而不是使用printf的原因。

(gdb)

0xb7fff419 in __kernel_vsyscall ()

(gdb)

Hello World!

我们发现Hello World!打印位置的上一行代码的执行位置为0xb7fff419。我们查看此处的反汇编代码。

(gdb)disassemble

Dump of assembler code for function __kernel_vsyscall:

   0xb7fff414 <+0>:  push   %ecx

   0xb7fff415 <+1>:  push   %edx

   0xb7fff416 <+2>:  push   %ebp

   0xb7fff417 <+3>:  mov    %esp,%ebp

   0xb7fff419 <+5>:  sysenter

   0xb7fff41b <+7>:  nop

   0xb7fff41c <+8>:  nop

   0xb7fff41d <+9>:  nop

   0xb7fff41e <+10>: nop

   0xb7fff41f <+11>: nop

   0xb7fff420 <+12>: nop

   0xb7fff421 <+13>: nop

   0xb7fff422 <+14>: int    $0x80

=> 0xb7fff424 <+16>: pop    %ebp

   0xb7fff425 <+17>: pop    %edx

   0xb7fff426 <+18>: pop    %ecx

   0xb7fff427 <+19>: ret   

End of assembler dump.

我们惊奇的发现，地址0xb7fff419正是指向sysenter指令的位置！这里便是系统调用的入口。如果想了解这里为什么不是int 0x80指令，请参考文章《Linux 2.6 对新型 CPU 快速系统调用的支持》。或者参考Linus在邮件列表里的文章《Intel P6 vs P7 system call performance》。

系统调用的位置已经是printf函数调用的末端了，我们只需要按照函数调用关系便能得到printf的调用轨迹了。

(gdb)backtrace

#0  0xb7fff424 in __kernel_vsyscall ()

#1  0x080588b2 in __write_nocancel ()

#2  0x0806fa11 in _IO_new_file_write ()

#3  0x0806f8ed in new_do_write ()

#4  0x080708dd in _IO_new_do_write ()

#5  0x08070aa5 in _IO_new_file_overflow ()

#6  0x08049a37 in puts ()

#7  0x08048eb9 in main () at main.c:4

我们发现系统调用前执行的函数是__write_nocancel，它执行了系统调用__write！

三、printf源码阅读

虽然我们找到了Hello World的printf调用轨迹，但是仍然无法看到函数的源码。跟踪反汇编代码不是个好主意，最好的方式是直接阅读glibc的源代码！我们可以从官网下载最新的glibc源代码（glibc-2.18）进行阅读分析，或者直接访问在线源码分析网站LXR。然后按照调用轨迹的的逆序查找函数的调用点。

1.puts 调用 _IO_new_file_xsputn

具体的符号转化关系为：_IO_puts => _IO_sputn => _IO_XSPUTN => __xsputn => _IO_file_xsputn => _IO_new_file_xsputn

$cat ./libio/ioputs.c

int

_IO_puts (str)

     const char *str;

{

  int result = EOF;

  _IO_size_t len = strlen (str);

  _IO_acquire_lock (_IO_stdout);

 

  if ((_IO_vtable_offset (_IO_stdout) != 0

       || _IO_fwide (_IO_stdout, -1) == -1)

      && _IO_sputn (_IO_stdout, str, len) == len

      && _IO_putc_unlocked ('\n', _IO_stdout) != EOF)

    result = MIN (INT_MAX, len + 1);

 

  _IO_release_lock (_IO_stdout);

  return result;

}

 

#ifdef weak_alias

weak_alias (_IO_puts, puts)

#endif

这里注意weak_alias宏的含义，即将puts绑定到符号_IO_puts，并且puts符号为weak类型的。这也就解释了puts符号被解析为_IO_puts的真正原因。

2._IO_new_file_xsputn 调用 _IO_new_file_overflow

具体的符号转化关系为：_IO_new_file_xsputn => _IO_OVERFLOW => __overflow => _IO_new_file_overflow

 
$cat ./libio/fileops.c

_IO_size_t

_IO_new_file_xsputn (f, data, n)

     _IO_FILE *f;

     const void *data;

     _IO_size_t n;

{

 ...

  if (to_do + must_flush > 0)

    {

      _IO_size_t block_size, do_write;

      /* Next flush the (full) buffer. */

      if (_IO_OVERFLOW (f, EOF) == EOF)

    /* If nothing else has to be written or nothing has been written, we

       must not signal the caller that the call was even partially

       successful.  */

    return (to_do == 0 || to_do == n) ? EOF : n - to_do;

...

3._IO_new_file_overflow 调用 _IO_new_do_write

具体的符号转化关系为：_IO_new_file_overflow =>_IO_do_write =>_IO_new_do_write

 
$cat ./libio/fileops.c

int

_IO_new_file_overflow (f, ch)

      _IO_FILE *f;

      int ch;

{

 ...

  if (INTUSE(_IO_do_write) (f, f->_IO_write_base,

  f->_IO_write_ptr - f->_IO_write_base) == EOF)

  return EOF;

  return (unsigned char) ch;

}

4. _IO_new_do_write 调用 new_do_write

具体的符号转化关系为：_IO_new_do_write => new_do_write

$cat ./libio/fileops.c

int

_IO_new_do_write (fp, data, to_do)

     _IO_FILE *fp;

     const char *data;

     _IO_size_t to_do;

{

  return (to_do == 0

      || (_IO_size_t) new_do_write (fp, data, to_do) == to_do) ? 0 : EOF;

}

5. new_do_write调用 _IO_new_file_write

具体的符号转化关系为：new_do_write =>_IO_SYSWRITE => __write() => write() => _IO_new_file_write

$cat ./libio/fileops.c

_IO_size_t

new_do_write (fp, data, to_do)

_IO_FILE *fp;

const char *data;

_IO_size_t to_do;

{

 ...

  count = _IO_SYSWRITE (fp, data, to_do);

  if (fp->_cur_column && count)

  fp->_cur_column = INTUSE(_IO_adjust_column) (fp->_cur_column - 1, data, count) + 1;

 ...

}

6. _IO_new_file_write调用 write_nocancel

具体的符号转化关系为：_IO_new_file_write=>write_not_cancel => write_nocancel

$cat ./libio/fileops.c

_IO_ssize_t

_IO_new_file_write (f, data, n)

_IO_FILE *f;

const void *data;

_IO_ssize_t n;

{

  _IO_ssize_t to_do = n;

  while (to_do > 0)

  {

    _IO_ssize_t count = (__builtin_expect (f->_flags2& _IO_FLAGS2_NOTCANCEL, 0)? write_not_cancel (f->_fileno, data, to_do): write (f->_fileno, data, to_do));

...

}

7. write_nocancel 调用 linux-gate.so::__kernel_vsyscall

具体的符号转化关系为：write_nocancel => INLINE_SYSCALL  => INTERNAL_SYSCALL =>__kernel_vsyscall

注意 linux-gate.so在磁盘上并不存在，它是内核镜像中的特定页，由内核编译生成。关于它的更多信息，可以参考文章《linux-gate.so技术细节》和《What is linux-gate.so.1?》。

四、总结

本文从printf(“Hello World !\n”)谈起，按照编译器工作的流程发掘了printf函数的代码位置。然后使用gdb反向跟踪的方式找到了printf函数的调用轨迹，最后借助于glibc源代码描述了printf函数的详细调用过程。相信这些内容会给大家带来一个更加清晰的printf函数，透过对printf函数的实现机制的解析，更可以加深对计算机工作机制的理解。希望本文对你有所帮助。
