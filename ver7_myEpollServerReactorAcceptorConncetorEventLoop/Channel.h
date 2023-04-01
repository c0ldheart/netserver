#pragma once
#include <functional>
#include <sys/epoll.h>
class EventLoop;
class Channel {
private:
    EventLoop *loop;
    int fd;
    uint32_t events;
    uint32_t revents;
    bool inEpoll;

    std::function<void()> callback;

public:
    Channel(EventLoop* _loop, int _fd);
    ~Channel();

    void enableReading();

    int getFd();
    uint32_t getEvents();
    uint32_t getRevents();
    bool getInEpoll();
    void setInEpoll();

    // void setEvents(uint32_t);
    void setRevents(uint32_t);

    // 回调函数
    // 设置该Channel的回调函数
    void set_callback(std::function<void()>);
    
    // 执行回调函数
    void handle_event();
};
