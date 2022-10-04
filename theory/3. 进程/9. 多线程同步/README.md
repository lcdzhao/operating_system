![sync](README.assets/sync.png)

- [深入理解Linux线程同步](https://blog.csdn.net/orangeboyye/article/details/125468728) —— 建议只看该文章的 线程防同步方法 这一部分
- [MESI 协议](https://www.cnblogs.com/yanlong300/p/8986041.html)
- [聊聊LOCK指令](https://albk.tech/%E8%81%8A%E8%81%8ACPU%E7%9A%84LOCK%E6%8C%87%E4%BB%A4.html)
- [一文解决内存屏障（含volatile原理与CAS原理）](https://monkeysayhi.github.io/2017/12/28/%E4%B8%80%E6%96%87%E8%A7%A3%E5%86%B3%E5%86%85%E5%AD%98%E5%B1%8F%E9%9A%9C/)
> 
> JMM （Java Memory Model ）Java内存模型是一个语言级别的内存模型抽象，它屏蔽了底层硬件实现内存一致性需求的差异，提供了对上层的统一的接口来提供保证内存一致性的编程能力。
> 
> Java作为一个跨平台的语言，Java内存模型作为一个中间层模型，它适配不同的底层硬件系统，设计一个中间层模型来屏蔽底层的硬件差异，给上层的开发者提供一个一致的使用接口，它为开发者屏蔽了底层的硬件实现细节，支持大部分的主流硬件平台。
> 
> 例：在 JVM 层面上时: 
> - 在每个volatile写操作的前面插入一个StoreStore屏障。
> - 在每个volatile写操作的后面插入一个StoreLoad屏障。
> - 在每个volatile读操作的后面插入一个LoadLoad屏障。
> - 在每个volatile读操作的后面插入一个LoadStore屏障。
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
> 在另外一些平台上，JVM使用 mfence 或者 LOCK 代替sfence与lfence，实现更强的语义。

- [悲观锁和乐观锁](https://mp.weixin.qq.com/s?__biz=MzkwMDE1MzkwNQ==&mid=2247496062&idx=1&sn=c04e0b83f38c45d06538ebac69529ee1&source=41#wechat_redirect)
- [自旋锁](https://www.cnblogs.com/cxuanBlog/p/11679883.html)
- [全面理解Java内存模型(JMM)及volatile关键字](https://blog.csdn.net/javazejian/article/details/72772461?spm=1001.2014.3001.5506)
