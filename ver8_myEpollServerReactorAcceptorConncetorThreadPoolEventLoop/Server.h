#pragma once
#include <map>
#include "EventLoop.h"
#include "Acceptor.h"
#include "Socket.h"
#include "Connection.h"
class Server
{
private:
    EventLoop *loop;
    Acceptor *acceptor;
public:
    std::map<int, Connection*> connections;
    Server(EventLoop*);
    ~Server();

    void new_connection(Socket *serv_sock);
    void delete_connection(Socket *serv_sock);
};