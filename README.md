# LFServer - 高性能 C++ 服务框架
从零自研的 Linux C++ 服务框架，集成 Reactor 网络库、线程池、内存池、时间轮和异步日志。

## ✨ 核心特性
*   **网络层**：Reactor + epoll，非阻塞 I/O，支持 Keep-Alive 长连接
*   **内存池**：多级定长池 + 自由链表，O(1) 分配与回收
*   **线程池**：支持 `std::future` 异步返回值，线程安全
*   **定时器**：时间轮，O(1) 插入与删除
*   **日志系统**：双缓冲异步日志，前端无锁
*   **HTTP 服务**：GET/POST 解析、静态文件、命令行配置、优雅退出

## 🚀 快速开始
```bash
git clone https://github.com/whn121/LFServer.git
cd LFServer
mkdir build && cd build
cmake .. && make
mkdir www && echo '<h1>Hello LFServer!</h1>' > www/index.html
./lfserver -p 8080 -t 4 -w ./www
