# 线程的状态有哪几种
这个问题相信很多人在面试中都被问过，看起来是一个非常简单的问题，但是实际上这个问题并没有表面上这么简单，因为这个问题的答案从不同层次来回答都是不一样的。

## 操作系统源码层面
操作系统的代码直接对接底层硬件，其运行在业务代码与硬件之间（在《Unix编程艺术》中被称为胶和层），故其在源码层面定义的线程状态侧重于如何在这个胶和层描述清楚一个线程，故其几个核心的状态为：

- Running
- Interruptable
- Uninterruptable
- Stop
- Zombie

## 教科书层面
教科书层面更倾向于给读者将一个线程的运行过程描述清楚，故其更加偏向于容易理解，其定义的状态与操作系统源码中定义的线程状态并不一一对应，具体为：

- Created
- Ready
- Running
- Blocked
- Terminate


## 高级语言层面（以JAVA为例）
高级语言层面，以Java为例，其为操作系统与程序员之间的桥梁，即要能够与操作系统的状态有一定的对应关系，又要屏蔽掉一些操作系统的复杂度，让程序员更易理解，故其与操作系统源码中定义的线程状态也不一一对应，具体为：

- New
- Running （包含操作系统中的Running及IO产生的Interruptable与Uninterruptable，由于在java层面并不关心操作系统层面的资源问题，仅关心语言层面定义的Blocked（锁阻塞）及Wait、Time_wait，故操作系统IO产生的Interruptable与Uninterruptable被Java视为Running）
- Wait
- Timed_wait
- Blocked （未获取到Java语言层面的锁，和操作系统层管理的硬件资源之间没有直接关系）
- Terminate
