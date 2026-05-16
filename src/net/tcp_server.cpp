#include "net/tcp_server.hpp"
#include "log/async_logger.hpp"
#include <functional>

TcpServer::TcpServer(EventLoop* loop, const std::string& name,
                     const struct sockaddr_in& listenAddr)
    : loop_(loop), name_(name), acceptor_(loop, listenAddr) {
    acceptor_.setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
    AsyncLogger::instance().log("[INFO] TcpServer created: " + name_);
}

TcpServer::~TcpServer() {
    for (auto& pair : connections_) {
        auto conn = pair.second;
        conn->getLoop()->runInLoop([conn] { conn->connectDestroyed(); });
    }
}

void TcpServer::start() {
    acceptor_.listen();
    AsyncLogger::instance().log("[INFO] Server " + name_ + " started.");
}

void TcpServer::newConnection(int sockfd, const struct sockaddr_in& peerAddr) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%s-%d", name_.c_str(), nextConnId_++);
    std::string connName = buf;

    AsyncLogger::instance().log("[INFO] New connection: " + connName);

    auto conn = std::make_shared<TcpConnection>(loop_, sockfd, connName);
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    conn->connectEstablished();
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    connections_.erase(conn->name());
    AsyncLogger::instance().log("[INFO] Connection " + conn->name() + " removed.");
    loop_->runInLoop([conn] { conn->connectDestroyed(); });
}
