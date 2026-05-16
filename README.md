
# LFServer - 高性能 C++ 服务框架

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)](https://en.cppreference.com/w/)
[![Linux](https://img.shields.io/badge/Linux-epoll-green)]()

**2000+ 行从零自研**，集成 Reactor + 内存池 + 线程池 + 时间轮 + 异步日志的高性能服务框架。

---

## 🔥 核心亮点

| 模块 | 技术 | 面试必问 |
|------|------|----------|
| **网络** | epoll + Reactor + 非阻塞IO | LT/ET 模式？EAGAIN 处理？ |
| **内存** | 多级定长池 + 自由链表 | O(1) 分配？碎片如何控制？ |
| **并发** | 线程池 + future 异步返回 | 优雅关闭？work stealing？ |
| **定时** | 时间轮 O(1) | 为什么不用红黑树？ |
| **日志** | 双缓冲异步无锁 | 双缓冲如何交换？ |

---

## 📊 压测数据（wrk, 4核8G）

```
QPS: 56,312  |  平均延迟: 1.98ms  |  并发: 5000+
```

---

## 🚀 30秒跑起来

```bash
git clone https://github.com/whn121/LFServer.git && cd LFServer
mkdir build && cd build && cmake .. && make -j
echo '<h1>Hello!</h1>' > ../www/index.html
./lfserver -p 8080 -t 4 -w ../www
# 浏览器打开 http://localhost:8080
```

---

## 🧩 架构一览

```
EventLoop (epoll_wait)
  ├── Acceptor (listenfd → accept4)
  ├── TcpConnection × N (connfd → handleRead/Write)
  │     └── 投递到线程池 → HttpParser → send
  ├── 时间轮 (管理超时连接)
  └── 异步日志 (双缓冲刷盘)
        ↑
     内存池 (所有对象分配)
```

---

## 📁 核心文件

```
include/net/      Reactor 核心 (EventLoop, Channel, TcpConnection)
include/pool/     内存池 & 线程池
include/timer/    时间轮定时器
include/log/      双缓冲异步日志
include/http/     HTTP 解析器 & 服务器

