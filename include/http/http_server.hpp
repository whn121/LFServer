#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "../net/tcp_server.hpp"
#include "../pool/thread_pool.hpp"
#include "../timer/timer.hpp"
#include "../log/async_logger.hpp"
#include "http_parser.hpp"
#include <sstream>
#include <fstream>
#include <cstring>
#include <atomic>
#include <iostream>
#include <netinet/in.h>

class HttpServer {
public:
    using TcpConnectionPtr = TcpConnection::TcpConnectionPtr;  // 简写别名

    HttpServer(EventLoop* loop, const std::string& name,
               uint16_t port, size_t thread_num = 4,
               const std::string& www_root = "./www")
        : server_(loop, name, createAddr(port)),
          thread_pool_(thread_num),
          www_root_(www_root) {

        AsyncLogger::instance().log("[INFO] HttpServer initializing, port=" +
                                    std::to_string(port) + ", threads=" +
                                    std::to_string(thread_num));

        server_.setConnectionCallback([this](const TcpConnectionPtr& conn) {
            total_connections_++;
            LOG_INFO("New HTTP connection: " + conn->name() +
                     " (total=" + std::to_string(total_connections_) + ")");

            // 60 秒超时
            uint64_t timer_id = timer_.add_timer(60000, [conn]() {
                conn->send("HTTP/1.1 408 Request Timeout\r\n"
                           "Content-Length: 0\r\n\r\n");
                conn->getLoop()->runInLoop([conn] { conn->connectDestroyed(); });
            });
            conn->setContext(timer_id);
        });

        server_.setMessageCallback([this](const TcpConnectionPtr& conn,
                                          std::string& buf) {
            // 重置定时器
            auto old_id = std::any_cast<uint64_t>(conn->getContext());
            timer_.cancel_timer(old_id);
            uint64_t new_id = timer_.add_timer(60000, [conn]() {
                conn->send("HTTP/1.1 408 Request Timeout\r\n"
                           "Content-Length: 0\r\n\r\n");
                conn->getLoop()->runInLoop([conn] { conn->connectDestroyed(); });
            });
            conn->setContext(new_id);

            // 投递到线程池
            std::string request_data = buf;
            thread_pool_.enqueue([this, conn, request_data]() {
                handleRequest(conn, request_data);
            });
        });
    }

    void start() {
        server_.start();
        LOG_INFO("HTTP Server started on port " +
                 std::to_string(ntohs(server_.listenAddr().sin_port)));
        std::cout << "========================================" << std::endl;
        std::cout << "  LFServer HTTP Server Started!" << std::endl;
        std::cout << "  Port: " << ntohs(server_.listenAddr().sin_port) << std::endl;
        std::cout << "  Workers: " << thread_pool_.worker_count() << std::endl;
        std::cout << "  WWW Root: " << www_root_ << std::endl;
        std::cout << "========================================" << std::endl;
    }

    void print_stats() const {
        std::cout << "[STATS] connections=" << total_connections_
                  << " requests=" << total_requests_ << std::endl;
    }

    TimerWheel& timer() { return timer_; }

private:
    void handleRequest(const TcpConnectionPtr& conn, const std::string& raw) {
        total_requests_++;

        HttpParser parser;
        HttpRequest req;
        if (!parser.parse(raw.data(), raw.size(), req)) {
            conn->send("HTTP/1.1 400 Bad Request\r\n"
                       "Content-Length: 0\r\n\r\n");
            return;
        }

        std::string response;
        if (req.method == "GET" || req.method == "HEAD") {
            std::string path = (req.path == "/") ? "/index.html" : req.path;
            std::string filepath = www_root_ + path;
            std::ifstream file(filepath);
            if (file) {
                std::stringstream ss;
                ss << file.rdbuf();
                std::string content = ss.str();
                std::string mime = getMimeType(path);

                response = "HTTP/1.1 200 OK\r\n";
                response += "Content-Type: " + mime + "\r\n";
                response += "Content-Length: " + std::to_string(content.size()) + "\r\n";
                response += "Connection: Keep-Alive\r\n";
                response += "Server: LFServer/1.0\r\n\r\n";
                if (req.method == "GET") response += content;
            } else {
                std::string body = "<h1>404 Not Found</h1><p>" + path + "</p>";
                response = "HTTP/1.1 404 Not Found\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: " + std::to_string(body.size()) + "\r\n"
                           "Connection: Keep-Alive\r\n\r\n" + body;
            }
        } else if (req.method == "POST") {
            std::string body = "{\"status\":\"ok\",\"path\":\"" + req.path + "\"}";
            response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: application/json\r\n"
                       "Content-Length: " + std::to_string(body.size()) + "\r\n"
                       "Connection: Keep-Alive\r\n\r\n" + body;
        } else {
            response = "HTTP/1.1 405 Method Not Allowed\r\n"
                       "Content-Length: 0\r\n\r\n";
        }

        conn->send(response);
    }

    std::string getMimeType(const std::string& path) {
        auto ends_with = [](const std::string& s, const std::string& suffix) {
            return s.size() >= suffix.size() &&
                   s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
        };
        if (ends_with(path, ".html")) return "text/html";
        if (ends_with(path, ".css"))  return "text/css";
        if (ends_with(path, ".js"))   return "application/javascript";
        if (ends_with(path, ".json")) return "application/json";
        if (ends_with(path, ".png"))  return "image/png";
        if (ends_with(path, ".jpg") || ends_with(path, ".jpeg")) return "image/jpeg";
        if (ends_with(path, ".gif"))  return "image/gif";
        if (ends_with(path, ".ico"))  return "image/x-icon";
        return "text/plain";
    }

    sockaddr_in createAddr(uint16_t port) {
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        return addr;
    }

    TcpServer server_;
    ThreadPool thread_pool_;
    TimerWheel timer_;
    std::string www_root_;
    std::atomic<size_t> total_connections_{0};
    std::atomic<size_t> total_requests_{0};
};

#endif
