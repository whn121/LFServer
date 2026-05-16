#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include "channel.hpp"
#include "eventloop.hpp"
#include <functional>
#include <memory>
#include <string>
#include <any>

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, std::string&)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

    TcpConnection(EventLoop* loop, int sockfd, const std::string& name);
    ~TcpConnection();

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    void connectEstablished();
    void connectDestroyed();
    void send(const std::string& message);

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }

    // 上下文存储（用于存放 timer_id 等）
    void setContext(const std::any& context) { context_ = context; }
    const std::any& getContext() const { return context_; }

private:
    void handleRead();
    void handleWrite();
    void handleClose();
    void sendInLoop(const std::string& message);

    EventLoop* loop_;
    int sockfd_;
    std::string name_;
    Channel channel_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    std::string inputBuffer_;
    std::string outputBuffer_;
    std::any context_;
};

#endif
