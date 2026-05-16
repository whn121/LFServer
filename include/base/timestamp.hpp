#ifndef TIMESTAMP_HPP
#define TIMESTAMP_HPP

#include <chrono>
#include <string>
#include <cstring>

class Timestamp {
public:
    static std::string now() {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) % 1000;
        return std::string(buf) + "." + std::to_string(ms.count());
    }
};

#endif
