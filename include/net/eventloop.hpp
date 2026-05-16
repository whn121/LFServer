#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <vector>
#include <mutex>
#include <sys/epoll.h>
#include "base/nocopyable.hpp"

class Channel;

class EventLoop : nocopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    void runInLoop(Functor cb);   // 在I/O线程执行任务
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    bool isInLoopThread() const { return threadId_ == std::this_thread::get_id(); }

private:
    void wakeup();                // 唤醒 epoll_wait
    void handleWakeup();
    void doPendingFunctors();

    bool looping_;
    std::atomic<bool> quit_;
    int epollfd_;
    int wakeupFd_;                // 用于唤醒的事件 fd
    std::unique_ptr<Channel> wakeupChannel_;
    std::vector<epoll_event> events_;
    std::thread::id threadId_;
    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_;
};

#endif
