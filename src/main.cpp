#include "http/http_server.hpp"
#include "net/eventloop.hpp"
#include "log/async_logger.hpp"
#include "timer/timer.hpp"
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

// 优雅退出
static EventLoop* g_loop = nullptr;
static HttpServer* g_server = nullptr;

void signal_handler(int sig) {
    std::cout << "\n[INFO] Received signal " << sig << ", shutting down..." << std::endl;
    if (g_loop) g_loop->quit();
}

struct Config {
    uint16_t port = 8080;
    size_t thread_num = 4;
    std::string log_path = "lfserver.log";
    std::string www_root = "./www";
};

Config parse_args(int argc, char* argv[]) {
    Config config;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-p" && i + 1 < argc) {
            config.port = static_cast<uint16_t>(std::stoi(argv[++i]));
        } else if (arg == "-t" && i + 1 < argc) {
            config.thread_num = std::stoi(argv[++i]);
        } else if (arg == "-l" && i + 1 < argc) {
            config.log_path = argv[++i];
        } else if (arg == "-w" && i + 1 < argc) {
            config.www_root = argv[++i];
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: lfserver [options]\n"
                      << "  -p <port>      Port (default 8080)\n"
                      << "  -t <threads>   Thread pool size (default 4)\n"
                      << "  -l <path>      Log file path (default lfserver.log)\n"
                      << "  -w <path>      WWW root directory (default ./www)\n"
                      << "  -h             Show this help\n";
            exit(0);
        }
    }
    return config;
}

int main(int argc, char* argv[]) {
    Config config = parse_args(argc, argv);

    // 初始化日志
    AsyncLogger::instance().init(config.log_path);

    // 注册信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    EventLoop loop;
    g_loop = &loop;

    HttpServer server(&loop, "HttpServer", config.port, config.thread_num, config.www_root);
    g_server = &server;
    server.start();

    LOG_INFO("LFServer started successfully");

    // 定时打印统计（每 10 秒）
    size_t tick_count = 0;
    server.timer().add_timer(10000, [&server, &tick_count]() {
        server.print_stats();
        // 重新注册下一次统计
        tick_count++;
    });

    loop.loop();

    LOG_INFO("LFServer stopped");
    std::cout << "Server stopped. Bye!" << std::endl;
    return 0;
}
