#include "net/acceptor.hpp"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

static int createNonblockingSocket() {
    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    return fd;
}

Acceptor::Acceptor(EventLoop* loop, const struct sockaddr_in& listenAddr)
    : loop_(loop),
      listenfd_(createNonblockingSocket()),
      listenAddr_(listenAddr),
      acceptChannel_(loop, listenfd_) {
    int on = 1;
    ::setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (::bind(listenfd_, (struct sockaddr*)&listenAddr, sizeof(listenAddr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    ::close(listenfd_);
}

void Acceptor::listen() {
    if (::listen(listenfd_, SOMAXCONN) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
    struct sockaddr_in peerAddr;
    socklen_t addrLen = sizeof(peerAddr);
    int connfd = ::accept4(listenfd_, (struct sockaddr*)&peerAddr, &addrLen,
                           SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        }
    } else {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("accept4");
        }
    }
}
