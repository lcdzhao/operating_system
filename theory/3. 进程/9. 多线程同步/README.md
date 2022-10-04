![sync](README.assets/sync.png)

- [深入理解Linux线程同步](https://blog.csdn.net/orangeboyye/article/details/125468728) —— 建议只看该文章的 线程防同步方法 这一部分
- [MESI 协议](https://www.cnblogs.com/yanlong300/p/8986041.html)
> CPU提供了三个汇编指令串行化运行读写指令达到实现保证读写有序性的目的：
> - SFENCE：在该指令前的写操作必须在该指令后的写操作前完成
> - LFENCE：在该指令前的读操作必须在该指令后的读操作前完成
> - MFENCE：在该指令前的读写操作必须在该指令后的读写操作前完成
> 
> Java的volatile在实现层面用的不是fence族屏障，而是lock。lock是如何实现屏障效果的呢？JVM为什么不用fence族呢？这两个问题在下面的几篇文章中解答：
> 
> - [正确认识volatile](https://mp.weixin.qq.com/s/wK3n42QNUO9Qp2Qql4Hslg)
> - [深入讲解线程工作内存](https://mp.weixin.qq.com/s/CQ3eX82Mc2tU2iYccMSPCg)

- [volatile底层原理详解](https://zhuanlan.zhihu.com/p/133851347)
- [CAS原理](https://www.jianshu.com/p/ab2c8fce878b)
- [悲观锁和乐观锁](https://mp.weixin.qq.com/s?__biz=MzkwMDE1MzkwNQ==&mid=2247496062&idx=1&sn=c04e0b83f38c45d06538ebac69529ee1&source=41#wechat_redirect)
- [自旋锁](https://www.cnblogs.com/cxuanBlog/p/11679883.html)
- [全面理解Java内存模型(JMM)及volatile关键字](https://blog.csdn.net/javazejian/article/details/72772461?spm=1001.2014.3001.5506)
