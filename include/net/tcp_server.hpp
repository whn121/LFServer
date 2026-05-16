#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include "eventloop.hpp"
#include "acceptor.hpp"
#include "tcp_connection.hpp"
#include <map>
#include <string>
#include <functional>
#include <netinet/in.h>

class TcpServer : nocopyable {
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, std::string&)>;

    TcpServer(EventLoop* loop, const std::string& name, const struct sockaddr_in& listenAddr);
    ~TcpServer();

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void start();

    const sockaddr_in& listenAddr() const { return acceptor_.listenAddr(); }

private:
    void newConnection(int sockfd, const struct sockaddr_in& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    std::string name_;
    Acceptor acceptor_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    int nextConnId_ = 1;
    std::map<std::string, TcpConnectionPtr> connections_;
};

#endif
