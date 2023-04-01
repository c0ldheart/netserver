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
    std::function<void()> call_back_func_new_connection = std::bind(&Acceptor::acceptConnection, this);
    accept_channel->set_callback(call_back_func_new_connection);
    accept_channel->enableReading();
}

Acceptor::~Acceptor(){
    delete socket_listen;
    delete address_server;
    delete accept_channel;
}

// 执行属于Accept的处理函数
void Acceptor::acceptConnection(){
    newConnectionCallback(socket_listen);
}

// 为Accept提供处理函数
void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> _cb){
    newConnectionCallback = _cb;
}