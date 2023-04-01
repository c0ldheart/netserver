#include "Acceptor.h"
#include "Channel.h"
#include "InetAddress.h"
#include "Server.h"
#include "Socket.h"
#include <functional>
Acceptor::Acceptor(EventLoop* _loop)
    : loop(_loop)
{
    socket_listen = new Socket();
    address_server = new InetAddress("127.0.0.1", 1888);
    socket_listen->bind(address_server);
    socket_listen->listen();
    socket_listen->setnonblocking();

    // ep来自EventLoop了
    // Epoll* ep = new Epoll();
    // Channel *servChannel = new Channel(ep, socket_listen->get_fd());
    accept_channel = new Channel(loop, socket_listen->get_fd());

    // 给Channel注册来自Accept的处理函数
    std::function<void()> call_back_func_new_connector = std::bind(&Acceptor::acceptConnection, this);
    accept_channel->set_callback(call_back_func_new_connector);
    accept_channel->enableReading();
}

Acceptor::~Acceptor(){
    delete socket_listen;
    delete address_server;
    delete accept_channel;
}

// 执行属于Accept的处理函数: 建立listen socket
void Acceptor::acceptConnection(){
    InetAddress* address_client = new InetAddress();
    int sockfd_clinet = this->socket_listen->accept(address_client);
    Socket* socket_client = new Socket(sockfd_clinet);
    printf("new client fd %d! IP: %s Port: %d\n", socket_client->get_fd(), inet_ntoa(address_client->get_addr().sin_addr), ntohs(address_client->get_addr().sin_port));
    socket_client->setnonblocking();
    this->do_newConnectionCallback(socket_client);
    delete address_client;
}

// 为Accept提供处理函数
void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> _cb){
    do_newConnectionCallback = _cb;
}