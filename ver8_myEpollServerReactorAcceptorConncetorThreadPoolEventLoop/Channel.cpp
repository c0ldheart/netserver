#include "Channel.h"
#include "EventLoop.h"
#include "ThreadPool.h"
#include <unistd.h>
Channel::Channel(EventLoop *_loop, int _fd) : loop(_loop), fd(_fd), events(0), revents(0), inEpoll(false){

}

Channel::~Channel()
{
    if(fd != -1){
    close(fd);
    fd = -1;
    }
}

void Channel::enableReading(){
    events = EPOLLIN | EPOLLET;
    loop->updateChannel(this);
}

int Channel::getFd(){
    return fd;
}

uint32_t Channel::getEvents(){
    return events;
}
uint32_t Channel::getRevents(){
    return revents;
}

bool Channel::getInEpoll(){
    return inEpoll;
}

void Channel::setInEpoll(){
    inEpoll = true;
}

// void Channel::setEvents(uint32_t _ev){
//     events = _ev;
// }

void Channel::setRevents(uint32_t _ev){
    revents = _ev;
}

// 回调函数的注册和执行
void Channel::set_callback(std::function<void()> _call_back_function) {
    this->callback = _call_back_function;
}

void Channel::handle_event() {
    loop->addThread(this->callback);
    // this->callback();
}