#pragma once
#include <map>
#include "EventLoop.h"
#include "Acceptor.h"
#include "Socket.h"
#include "Connection.h"
#include "ThreadPool.h"
#include <vector>
class Server
{
private:
    EventLoop *main_reactor;
    Acceptor *acceptor;
    std::vector<EventLoop *> sub_reactors;
    ThreadPool* thpool;
public:
    std::map<int, Connection*> connections;
    Server(EventLoop*);
    ~Server();

    void new_connection(Socket *serv_sock);
    void delete_connection(int);

    void handle_read_event(int);
};