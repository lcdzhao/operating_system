# 操作系统的发展
Linux 0.11 是一个汇聚了无数人智慧结晶才诞生的产物，其已经是一个无比精妙的系统了。不过现在距离 Linux 0.11 的诞生已经过去几十年了，操作系统和硬件虽然没有什么彻头彻尾的变动，但是很多方面做的却比以前更加完美了，当然完美的代价是代码变得极其复杂，晦涩难懂，这里不展开源码讲解，我想大概谈一谈这些变动的大方向。

核心的大方向其实也就只有两个，一个是减少没有必要的资源消耗，二是虚拟化。

- 减少没有必要的资源消耗：操作系统的资源大的来讲，主要有：CPU 内存 磁盘 网络几个方面，下面我们来看看这一块的发展：

    - 减少 CPU 消耗：这块我们从硬件和软件方面分别来谈：
        - 硬件方面：
           - X86 将指令翻译为 精简指令集 再使用流水线；
           - MESI 协议 与 InvalidateQueue及WriteBuffer 的结合；
           - 更丰富的DMA模块。
           - 减少系统调用本身的消耗，从 INT 中断发起系统调用 到 sysent 发起系统调用。
           - ......

        - 软件方面：软件方面除了操作系统的代码本身更加卓越，其执行比以前更节省CPU资源以外，用户态可感知到的主要为：操作系统尽可能地减少了用户态代码完成一项任务所需要的调用系统调用的次数，关于减少系统调用次数，基本上下面有三种优化思路：
            - 第一种思路：减少没有必要进行系统调用，这一类大多借助 mmap，创建共享内存，将一部分内核空间的访问权限交给用户空间，仅把需要保护的操作保护起来。如：
                - 从 poll  select 到 epoll 
                - 从 信号量 到 futex（运行在用户态的线程仅在需要暂停或者继续时，才需要使用系统调用）
                - ......
            - 第二种思路：彻底不进行系统调用，这种思路比较激进，会让操作系统的隔离性变差，并丢失部分操作系统本身的功能，会直接影响到其他应用程序的运行，但是部分特殊场景下可以节省大量的CPU资源，如：DPDK结合LVS将LVS性能提升10倍左右。（不过未来这一部分场景中的很多应该会逐渐被eBPF所取代，eBPF即是第三种减少系统调用的方法）
            - 第三种思路：将一部分安全的用户态代码放入内核态执行，即为eBPF技术（被誉为操作系统近50年来最大的改动），facebook的基于 eBPF 的开源负载均衡器 katran 性能达到 dpvs 同一级别，同时又不丢失操作系统的隔离性和部分操作系统本身的功能。
    

     - 减少内存消耗：虚拟内存的诞生不光巧妙地实现了进程的内存隔离性，更帮助操作系统极大地节约了内存的使用。不过虚拟内存技术在Linux 0.11 中已经存在，也许是因为虚拟内存技术太过优秀，在用户态的内存分配方面，操作系统基本没有变化（当然C语言标准库的malloc函数内部可能有变化，不过这块不属于操作系统的内存分配，故其变化不包含在该篇文章中），有变化的只有内核态中内存分配，具体如下：
        - 内核从一开始内存分配至少一页，需要内核编程人员自己去分割这一页内存，变为可以根据不同大小的需求分配内存，发展出 buddy 内存管理系统，针对不同大小均可快速分配相应大小的内存，如：slab等等。



- 虚拟化：
    - 容器技术：
        - namespace
        - cgroup
        - chroot

    - 内核虚拟化
        - KVM
