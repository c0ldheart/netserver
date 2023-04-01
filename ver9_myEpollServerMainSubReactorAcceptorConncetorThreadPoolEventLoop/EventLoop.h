#pragma once
#include <functional>
#include "ThreadPool.h"
class Epoll;
class Channel;
class ThreadPoll;
class EventLoop
{
private:
    Epoll *ep;
    // ThreadPool *threadPool;
    bool quit;
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void updateChannel(Channel*);

    // void addThread(std::function<void()>);
};

