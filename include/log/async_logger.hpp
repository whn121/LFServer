#ifndef ASYNC_LOGGER_HPP
#define ASYNC_LOGGER_HPP

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <fstream>
#include <iostream>

class AsyncLogger {
public:
    static AsyncLogger& instance() {
        static AsyncLogger logger;
        return logger;
    }

    void init(const std::string& filepath, int flush_interval_ms = 1000) {
        filepath_ = filepath;
        flush_interval_ms_ = flush_interval_ms;
        running_ = true;
        backend_thread_ = std::thread(&AsyncLogger::backend_loop, this);
    }

    void log(const std::string& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        front_buffer_.push_back(msg);
        if (front_buffer_.size() >= BUFFER_THRESHOLD) {
            cv_.notify_one();
        }
    }

    ~AsyncLogger() {
        running_ = false;
        cv_.notify_all();
        if (backend_thread_.joinable()) backend_thread_.join();
    }

private:
    AsyncLogger() = default;

    void backend_loop() {
        std::ofstream file(filepath_, std::ios::app);
        while (running_) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, std::chrono::milliseconds(flush_interval_ms_), [this] {
                return !running_ || front_buffer_.size() >= BUFFER_THRESHOLD;
            });
            back_buffer_.swap(front_buffer_); // 交换双缓冲
            lock.unlock();

            for (const auto& msg : back_buffer_) {
                file << msg << std::endl;
            }
            back_buffer_.clear();
            file.flush();
        }
        // 最后一次刷新
        file.flush();
    }

    static constexpr size_t BUFFER_THRESHOLD = 100;

    std::string filepath_;
    int flush_interval_ms_ = 1000;
    std::atomic<bool> running_{false};
    std::thread backend_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<std::string> front_buffer_;
    std::vector<std::string> back_buffer_;
};

// 便捷宏
#define LOG_INFO(msg) AsyncLogger::instance().log("[INFO] " + std::string(msg))
#define LOG_ERROR(msg) AsyncLogger::instance().log("[ERROR] " + std::string(msg))

#endif
