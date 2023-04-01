#include "Channel.h"
#include "EventLoop.h"
#include "ThreadPool.h"
#include <unistd.h>
Channel::Channel(EventLoop* _loop, int _fd)
    : loop(_loop)
    , fd(_fd)
    , events(0)
    , ready(0)
    , inEpoll(false)
{
}

Channel::~Channel()
{
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

void Channel::handle_event()
{
    // loop->addThread(this->callback);
    // // this->callback();
    if (ready & (EPOLLIN | EPOLLPRI)) {
        readCallback();
    }
    if (ready & (EPOLLOUT)) {
        writeCallback();
    }
}

void Channel::enableRead(){
    events |= EPOLLIN | EPOLLPRI;
    loop->updateChannel(this);
}

void Channel::useET(){
    events |= EPOLLET;
    loop->updateChannel(this);
}
int Channel::getFd(){
    return fd;
}

uint32_t Channel::getEvents(){
    return events;
}
uint32_t Channel::getReady(){
    return ready;
}

bool Channel::getInEpoll(){
    return inEpoll;
}

void Channel::setInEpoll(bool _in){
    inEpoll = _in;
}

void Channel::setReady(uint32_t _ev){
    ready = _ev;
}

void Channel::setReadCallback(std::function<void()> _cb){
    readCallback = _cb;
}




// 回调函数的注册和执行
// void Channel::set_callback(std::function<void()> _call_back_function)
// {
//     this->callback = _call_back_function;
// }

