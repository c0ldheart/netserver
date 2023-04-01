#pragma once
#include <functional>
#include <sys/epoll.h>
class EventLoop;
class Channel {
private:
    EventLoop* loop;
    int fd;
    uint32_t events;

    bool inEpoll;
    uint32_t ready;
    std::function<void()> readCallback;
    std::function<void()> writeCallback;
    
public:
    Channel(EventLoop* _loop, int _fd);
    ~Channel();

    void enableRead();
    void useET();

    int getFd();

    void setInEpoll(bool _in = true);
    bool getInEpoll();

    // void setEvents(uint32_t);
    
    uint32_t getEvents();
    void setEvents(uint32_t);

    // 回调函数
    // 设置该Channel的回调函数

    void setReady(uint32_t);
    uint32_t getReady();

    void setReadCallback(std::function<void()>);

    // 执行回调函数
    void handle_event();
};
