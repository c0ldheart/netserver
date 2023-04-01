#include "Acceptor.h"
#include "Channel.h"
#include "InetAddress.h"
#include "Server.h"
#include "Socket.h"
#include <functional>
#include <memory>
Acceptor::Acceptor(EventLoop* _loop)
    : loop(_loop)
    , socket_listen(nullptr)
    , accept_channel(nullptr)
{
    socket_listen = new Socket();
    address_server = new InetAddress("127.0.0.1", 1888);
    socket_listen->bind(address_server);
    socket_listen->listen();
    // socket_listen->setnonblocking();

    // ep来自EventLoop了
    // Epoll* ep = new Epoll();
    // Channel *servChannel = new Channel(ep, socket_listen->get_fd());
    accept_channel = new Channel(loop, socket_listen->get_fd());
    // 给Channel注册来自Accept的处理函数
    std::function<void()> call_back_func_new_connector = std::bind(&Acceptor::acceptConnection, this);
    accept_channel->setReadCallback(call_back_func_new_connector); // 命名不合理
    accept_channel->enableRead();
}

Acceptor::~Acceptor()
{
    delete socket_listen;
    delete address_server;
    delete accept_channel;
}

// 执行属于Accept的处理函数: 建立listen socket
void Acceptor::acceptConnection()
{
    InetAddress *clnt_addr = new InetAddress();      
    Socket *clnt_sock = new Socket(socket_listen->accept(clnt_addr));      
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sock->get_fd(), clnt_addr->get_ip(), clnt_addr->get_port());
    clnt_sock->setnonblocking();  //新接受到的连接设置为非阻塞式
    do_newConnectionCallback(clnt_sock);
    delete clnt_addr;
    
}

// 为Accept提供处理函数
void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> _cb)
{
    do_newConnectionCallback = _cb;
}