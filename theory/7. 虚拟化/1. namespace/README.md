
## 文章讲解

- (Docker 基础技术之 Linux namespace 详解)[https://mp.weixin.qq.com/s/10HgkUE14wVI_RNmFdqkzA]

- (Docker 基础技术之 Linux namespace 源码分析)[https://mp.weixin.qq.com/s/czZsAmp6nTt6JPuOAytgcw]
## 视频讲解
视频讲解：https://www.bilibili.com/video/BV1oi4y1F7sb?spm_id_from=333.999.0.0&vd_source=afbe39567defad401c79f6fbb57691cf

视频实验：https://www.bilibili.com/video/BV1oi4y1F7sb?p=4&vd_source=afbe39567defad401c79f6fbb57691cf

## Docker 如何使用 namespace
> Docker uses a technology called namespaces to provide the isolated workspace called the container. When you run a container, Docker creates a set of namespaces for that container.
> 
> These namespaces provide a layer of isolation. Each aspect of a container runs in a separate namespace and its access is limited to that namespace.
