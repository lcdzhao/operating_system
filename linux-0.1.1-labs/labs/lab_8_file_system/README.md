# proc文件系统的实现
## 课程说明
本实验是 操作系统之外设与文件系统 - 网易云课堂 的配套实验，推荐大家进行实验之前先学习相关课程：

- L28 生磁盘的使用
- L29 用文件使用磁盘
- L30 文件使用磁盘的实现
- L31 目录与文件系统
- L32 目录解析代码实现

> Tips：点击上方文字中的超链接或者输入 https://mooc.study.163.com/course/1000002009#/info 进入理论课程的学习。 如果网易云上的课程无法查看，也可以看 Bilibili 上的 操作系统哈尔滨工业大学李治军老师。

## 实验目的

- 掌握虚拟文件系统的实现原理；
- 实践文件、目录、文件系统等概念。

## 实验内容
在 Linux 0.11 上实现 `procfs`（`proc` 文件系统）内的 `psinfo` 结点。当读取此结点的内容时，可得到系统当前所有进程的状态信息。例如，用 `cat` 命令显示 `/proc/psinfo` 的内容，可得到：
```shell
$ cat /proc/psinfo
pid    state    father    counter    start_time
0    1    -1    0    0
1    1    0    28    1
4    1    1    1    73
3    1    1    27    63
6    0    4    12    817
```

```shell
$ cat /proc/hdinfo
total_blocks:    62000;
free_blocks:    39037;
used_blocks:    22963;
...
```

`procfs` 及其结点要在内核启动时自动创建。

相关功能实现在 `fs/proc.c` 文件内。

## 实验提示
本实验文档在 Linux 0.11 上实现 `procfs`（`proc` 文件系统）内的 `psinfo` 结点。当读取 `psinfo` 结点的内容时，可得到系统当前所有进程的状态信息。

最后还给出来 `hdinfo` 结点实现的提示。

### procfs 简介
正式的 Linux 内核实现了 `procfs`，它是一个虚拟文件系统，通常被 `mount`（挂载） 到 `/proc` 目录上，通过虚拟文件和虚拟目录的方式提供访问系统参数的机会，所以有人称它为 “了解系统信息的一个窗口”。

这些虚拟的文件和目录并没有真实地存在在磁盘上，而是内核中各种数据的一种直观表示。虽然是虚拟的，但它们都可以通过标准的系统调用（`open()`、`read()` 等）访问。

例如，`/proc/meminfo` 中包含内存使用的信息，可以用 `cat` 命令显示其内容：

```shell
$ cat /proc/meminfo
MemTotal:       384780 kB
MemFree:         13636 kB
Buffers:         13928 kB
Cached:         101680 kB
SwapCached:        132 kB
Active:         207764 kB
Inactive:        45720 kB
SwapTotal:      329324 kB
SwapFree:       329192 kB
Dirty:               0 kB
Writeback:           0 kB
……
```

其实，Linux 的很多系统命令就是通过读取 `/proc` 实现的。例如 `uname -a` 的部分信息就来自 `/proc/version`，而 `uptime` 的部分信息来自 `/proc/uptime` 和 `/proc/loadavg`。

关于 `procfs` 更多的信息请访问：http://en.wikipedia.org/wiki/Procfs

### 基本思路

Linux 是通过文件系统接口实现 `procfs`，并在启动时自动将其 `mount` 到 `/proc` 目录上。

此目录下的所有内容都是随着系统的运行自动建立、删除和更新的，而且它们完全存在于内存中，不占用任何外存空间。

Linux 0.11 还没有实现虚拟文件系统，也就是，还没有提供增加新文件系统支持的接口。所以本实验只能在现有文件系统的基础上，通过打补丁的方式模拟一个 `procfs`。

Linux 0.11 使用的是 Minix 的文件系统，这是一个典型的基于 `inode` 的文件系统，《注释》一书对它有详细描述。它的每个文件都要对应至少一个 `inode`，而 `inode` 中记录着文件的各种属性，包括文件类型。文件类型有普通文件、目录、字符设备文件和块设备文件等。在内核中，每种类型的文件都有不同的处理函数与之对应。我们可以增加一种新的文件类型——`proc` 文件，并在相应的处理函数内实现 `procfs` 要实现的功能。



