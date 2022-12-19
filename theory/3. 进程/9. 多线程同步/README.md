![sync](README.assets/sync.png)

- # [ futex ](https://www.cnblogs.com/yysblog/archive/2012/11/03/2752728.html)
- # [从AQS到futex(三): glibc(NPTL)的mutex/cond实现](http://kexianda.info/2017/08/17/%E5%B9%B6%E5%8F%91%E7%B3%BB%E5%88%97-5-%E4%BB%8EAQS%E5%88%B0futex%E4%B8%89-glibc-NPTL-%E7%9A%84mutex-cond%E5%AE%9E%E7%8E%B0/) 
- # [柯贤达-并发系列,从AQS到futex完整解析,质量非常高](http://kexianda.info）
- # [MESI 协议](https://www.cnblogs.com/yanlong300/p/8986041.html)
> ### 文章纠错: 
> 
> #### 错误一
> Store Bufferes
> 
> 为了避免这种CPU运算能力的浪费，Store Bufferes被引入使用。处理器把它想要**写入到主存的值写到缓存（正确应为：写入到本地缓存的值写到 Store Bufferes ）**，然后继续去处理其他事情。当所有相关的失效确认（Invalidate Acknowledge）都接收到时，数据才会最终被提交。
> 
> #### 错误二
> Store Bufferes的风险
> 
> 第一: 就是处理器会尝试从存储缓存（Store buffer）中读取值，但它还没有进行提交。这个的解决方案称为Store Forwarding，它使得加载的时候，**如果存储缓存（正确应该再补上: Store buffer）中**存在，则进行返回。
> 
- # [StoreBuffer与Invalid Queue](https://blog.csdn.net/wll1228/article/details/107775976)
- # [聊聊LOCK指令](https://albk.tech/%E8%81%8A%E8%81%8ACPU%E7%9A%84LOCK%E6%8C%87%E4%BB%A4.html)
> ## 处理器如何实现原子操作
> 首先处理器会保证基本的内存操作的原子性，比如从内存读取或者写入一个字节是原子的，但对于读-改-写、或者是其它复杂的内存操作是不能保证其原子性的，又比如跨总线宽度、跨多个缓存行和夸页表> 的访问，这时候需要处理器提供总线锁和缓存锁两个机制来保证复杂的内存操作原子性
> 
> ### 总线锁
> LOCK#信号就是我们经常说到的总线锁，处理器使用LOCK#信号达到锁定总线，来解决原子性问题，当一个处理器往总线上输出LOCK#信号时，其它处理器的请求将被阻塞，此时该处理器此时独占共享内存。
> 
> 总线锁这种做法锁定的范围太大了，导致CPU利用率急剧下降，因为使用LOCK#是把CPU和内存之间的通信锁住了，这使得锁定时期间，其它处理器不能操作其内存地址的数据 ，所以总线锁的开销比较大。
> 
> ### 缓存锁
> 如果访问的内存区域已经缓存在处理器的缓存行中，P6系统和之后系列的处理器则不会声明LOCK#信号，它会对CPU的缓存中的缓存行进行锁定，在锁定期间，其它 CPU 不能同时缓存此数据，在修改之后，通过缓存一致性协议来保证修改的原子性，这个操作被称为“缓存锁”
> 
> ### 什么情况下使用总线锁(LOCK#)
> 当操作的数据不能被缓存在处理器内部，或操作的数据跨多个缓存行时，也会使用总线锁
> 因为从P6系列处理器开始才有缓存锁，所以对于早些处理器是不支持缓存锁定的，也会使用总线锁
> 
> ### 有些指令自带总线锁
> BTS、BTR、BTC 、XADD、CMPXCHG、ADD、OR等，这些指令操作的内存区域就会加锁，导致其它处理器不能同时访问它。
> 
> 在上面指令中的CMPXCHG就是JAVA里面CAS底层常用的指令，这个指令在执行的时候，会自动加总线锁保，导致其它 处理器不能同时访问，证其原子性。
> 
> ### LOCK#作用总结
> - 锁总线，其它CPU对内存的读写请求都会被阻塞，直到锁释放，因为锁总线的开销比较大，后来的处理器都采用锁缓存替代锁总线，在无法使用缓存锁的时候会降级使用总线锁
> - lock期间的写操作会回写已修改的数据到主内存，同时通过缓存一致性协议让其它CPU相关缓存行失效

- # [一文解决内存屏障（含volatile原理与CAS原理）](https://monkeysayhi.github.io/2017/12/28/%E4%B8%80%E6%96%87%E8%A7%A3%E5%86%B3%E5%86%85%E5%AD%98%E5%B1%8F%E9%9A%9C/)
> 既然有了MESI协议，是不是就不需要volatile的可见性语义了？当然不是，还有三个问题：
>
> - 并不是所有的硬件架构都提供了相同的一致性保证，JVM需要volatile统一语义（就算是MESI，也只解决CPU缓存层面的问题，没有涉及其他层面）。
> - 可见性问题不仅仅局限于CPU缓存内，JVM自己维护的内存模型中也有可见性问题。使用volatile做标记，可以解决JVM层面的可见性问题。
> - 如果不考虑真·重排序，MESI确实解决了CPU缓存层面的可见性问题；然而，真·重排序也会导致可见性问题。
> 
> 暂时第一个问题称为“内存可见性”问题，内存屏障解决了该问题。后文讨论。
> ## JMM
> JMM （Java Memory Model ）Java内存模型是一个语言级别的内存模型抽象，它屏蔽了底层硬件实现内存一致性需求的差异，提供了对上层的统一的接口来提供保证内存一致性的编程能力。
> 
> Java作为一个跨平台的语言，Java内存模型作为一个中间层模型，它适配不同的底层硬件系统，设计一个中间层模型来屏蔽底层的硬件差异，给上层的开发者提供一个一致的使用接口，它为开发者屏蔽了底层的硬件实现细节，支持大部分的主流硬件平台。
> 
> 下面以 volatile 为例。
> 
> ### 在 JMM 的抽象层面上时 
> - 在每个volatile写操作的前面插入一个StoreStore屏障。
> - 在每个volatile写操作的后面插入一个StoreLoad屏障。
> - 在每个volatile读操作的后面插入一个LoadLoad屏障。
> - 在每个volatile读操作的后面插入一个LoadStore屏障。
> 
> ### 在实际的实现层面上：
> 
> 如果硬件架构本身已经保证了内存可见性（如单核处理器、一致性足够的内存模型等），那么volatile就是一个空标记，不会插入相关语义的内存屏障。
> 
> 如果硬件架构本身不进行处理器重排序、有更强的重排序语义（能够分析多核间的数据依赖）、或在单核处理器上重排序，那么volatile就是一个空标记，不会插入相关语义的内存屏障。
> 
> 在X86架构的实现时，如果硬件架构本身没有保证内存可见性，JVM对volatile变量的处理如下：
> 
> - 在写volatile变量v之后，插入一个sfence。这样，sfence之前的所有store（包括写v）不会被重排序到sfence之后，sfence之后的所有store不会被重排序到sfence之前，禁用跨sfence的store重排序； 且sfence之前修改的值都会被写回缓存，并标记其他CPU中的缓存失效。
> - 在读volatile变量v之前，插入一个lfence。这样，lfence之后的load（包括读v）不会被重排序到lfence之前，lfence之前的load不会被重排序到lfence之后，禁用跨lfence的load重排序；且lfence之后，会首先刷新无效缓存，从而得到最新的修改值，与sfence配合保证内存可见性。
> 
> 在另外一些平台上，JVM使用 mfence 或者 LOCK 代替 sfence 与 lfence，实现更强的语义。

- # [悲观锁和乐观锁](https://mp.weixin.qq.com/s?__biz=MzkwMDE1MzkwNQ==&mid=2247496062&idx=1&sn=c04e0b83f38c45d06538ebac69529ee1&source=41#wechat_redirect)
- # [自旋锁](https://www.cnblogs.com/cxuanBlog/p/11679883.html)
- # [全面理解Java内存模型(JMM)及volatile关键字](https://blog.csdn.net/javazejian/article/details/72772461?spm=1001.2014.3001.5506)
- # 其他
> - ### [伪共享与缓存行填充-1](https://blog.csdn.net/qq_27680317/article/details/78486220)
> - ### [伪共享与缓存行填充-2](https://blog.51cto.com/u_13561855/4035624)

