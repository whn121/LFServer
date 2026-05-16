#ifndef TIMER_HPP
#define TIMER_HPP

#include <functional>
#include <unordered_map>
#include <vector>
#include <cstdint>

class TimerWheel {
public:
    using TimerCallback = std::function<void()>;
    struct TimerEntry {
        int64_t expiration;   // 到期时间（毫秒）
        TimerCallback cb;
    };

    explicit TimerWheel(int64_t tick_ms = 1000, int wheel_size = 60)
        : tick_ms_(tick_ms), wheel_size_(wheel_size), current_index_(0) {
        wheel_.resize(wheel_size_);
    }

    // 添加定时器，返回 timer_id
    uint64_t add_timer(int64_t delay_ms, TimerCallback cb) {
        int64_t expires = current_time_ms() + delay_ms;
        uint64_t id = next_id_++;
        timers_[id] = {expires, std::move(cb)};
        schedule(id, expires);
        return id;
    }

    void cancel_timer(uint64_t id) {
        auto it = timers_.find(id);
        if (it != timers_.end()) {
            timers_.erase(it);
            // 简单实现：不主动从 wheel 移除，tick 时检查是否有效
        }
    }

    // 每次 epoll_wait 后调用，处理到期定时器
    void tick() {
        int64_t now = current_time_ms();
        // 检查当前槽及之前因为精度可能错过的槽
        for (int i = 0; i < wheel_size_; ++i) {
            int idx = (current_index_ + i) % wheel_size_;
            auto& slot = wheel_[idx];
            auto it = slot.begin();
            while (it != slot.end()) {
                uint64_t id = *it;
                auto timer_it = timers_.find(id);
                if (timer_it == timers_.end()) {
                    // 已取消
                    it = slot.erase(it);
                    continue;
                }
                if (timer_it->second.expiration <= now) {
                    // 到期
                    timer_it->second.cb();
                    timers_.erase(timer_it);
                    it = slot.erase(it);
                } else {
                    ++it;
                }
            }
        }
        current_index_ = (current_index_ + 1) % wheel_size_;
    }

private:
    void schedule(uint64_t id, int64_t expires) {
        int64_t now = current_time_ms();
        int64_t delay = expires - now;
        if (delay < 0) delay = 0;
        size_t idx = (current_index_ + (delay / tick_ms_)) % wheel_size_;
        wheel_[idx].push_back(id);
    }

    int64_t current_time_ms() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch()).count();
    }

    int64_t tick_ms_;
    int wheel_size_;
    int current_index_;
    std::vector<std::vector<uint64_t>> wheel_;
    std::unordered_map<uint64_t, TimerEntry> timers_;
    uint64_t next_id_ = 1;
};

#endif
