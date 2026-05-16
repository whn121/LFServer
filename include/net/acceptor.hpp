#ifndef ACCEPTOR_HPP
#define ACCEPTOR_HPP

#include "channel.hpp"
#include "eventloop.hpp"
#include <functional>
#include <netinet/in.h>

class Acceptor : nocopyable {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const struct sockaddr_in&)>;

    Acceptor(EventLoop* loop, const struct sockaddr_in& listenAddr);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }
    void listen();

    const sockaddr_in& listenAddr() const { return listenAddr_; }

private:
    void handleRead();

    EventLoop* loop_;
    int listenfd_;
    sockaddr_in listenAddr_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
};

#endif
