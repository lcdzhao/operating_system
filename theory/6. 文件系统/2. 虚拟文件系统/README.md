
# 虚拟文件系统
## 基本概念
[点击跳转](https://zhuanlan.zhihu.com/p/402127017)

## 数据结构
![vfs-frame-look](README.assets/vfs-frame-look.png)

![VFS](README.assets/VFS.png)

## mount 源码分析
http://edsionte.com/techblog/archives/4389         


## open close 源码分析
https://www.cnblogs.com/zengyiwen/p/5755186.html

## read write 源码分析
- [read](https://github.com/lcdzhao/operating_system/blob/master/theory/6.%20%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/2.%20%E8%99%9A%E6%8B%9F%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/README.assets/Linux.Kernel.Read.Procedure.pdf)

- [write](https://github.com/lcdzhao/operating_system/blob/master/theory/6.%20%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/2.%20%E8%99%9A%E6%8B%9F%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/README.assets/Linux.Kernel.Write.Procedure.pdf)

## Linux内核的文件预读机制详细详解
- [Linux内核的文件预读机制详细详解](https://blog.csdn.net/kunyus/article/details/104620057)

其他可以参考的文章：

  - [Linux内核的文件预读readahead查看与设置](https://blog.51cto.com/u_15338523/3592323)
  
  - [Linux Cache 机制与预读](https://github.com/lcdzhao/operating_system/blob/master/theory/6.%20%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/2.%20%E8%99%9A%E6%8B%9F%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/README.assets/Linux.Kernel.Cache.pdf)



## Page Cache 源码分析
[Page Cache 刷脏源码分析](https://www.leviathan.vip/2019/06/01/Linux%E5%86%85%E6%A0%B8%E6%BA%90%E7%A0%81%E5%88%86%E6%9E%90-Page-Cache%E5%8E%9F%E7%90%86%E5%88%86%E6%9E%90/)

[Linux 缺页处理](https://www.leviathan.vip/2019/03/03/Linux%E5%86%85%E6%A0%B8%E6%BA%90%E7%A0%81%E5%88%86%E6%9E%90-%E5%86%85%E5%AD%98%E8%AF%B7%E9%A1%B5%E6%9C%BA%E5%88%B6/)


# 其他讲解方式
> 来源: https://ilinuxkernel.com/

    在块设备上的操作，涉及内核中的多个组成部分，如图1所示。假设一个进程使用系统调用read（）读取磁盘上的文件。下面步骤是内核响应进程读请求的步骤；

（1）系统调用read（）会触发相应的VFS（Virtual Filesystem Switch）函数，传递的参数有文件描述符和文件偏移量。

（2）VFS确定请求的数据是否已经在内存缓冲区中；若数据不在内存中，确定如何执行读操作。

（3）假设内核必须从块设备上读取数据，这样内核就必须确定数据在物理设备上的位置。这由映射层（Mapping Layer）来完成。

（4）此时内核通过通用块设备层（Generic Block Layer）在块设备上执行读操作，启动I/O操作，传输请求的数据。

（5）在通用块设备层之下是I/O调度层（I/O Scheduler Layer），根据内核的调度策略，对等待的I/O等待队列排序。

（6）最后，块设备驱动（Block Device Driver）通过向磁盘控制器发送相应的命令，执行真正的数据传输。

其中：

- 关于(1) (2) ，见:[Linux虚拟文件系统](https://github.com/lcdzhao/operating_system/blob/master/theory/6.%20%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/2.%20%E8%99%9A%E6%8B%9F%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/README.assets/Linux.Virtual.Filesystem.pdf)

- 关于(3)，见：[Linux内核文件Cache机制](https://github.com/lcdzhao/operating_system/blob/master/theory/6.%20%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/2.%20%E8%99%9A%E6%8B%9F%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/README.assets/Linux.Kernel.Cache.pdf)

- 关于(4)，见: [Linux通用块设备层](https://github.com/lcdzhao/operating_system/blob/master/theory/6.%20%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/2.%20%E8%99%9A%E6%8B%9F%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/README.assets/Linux.Generic.Block.Layer.pdf)

- 关于(5)，见：[Linux内核I/O调度层](https://github.com/lcdzhao/operating_system/blob/master/theory/6.%20%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/2.%20%E8%99%9A%E6%8B%9F%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F/README.assets/Linux.Kernel.IO.Scheduler.pdf)

- 关于(6)，这里就不展开讲解了。
