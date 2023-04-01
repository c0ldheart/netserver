#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include <vector>

EventLoop::EventLoop() : ep(nullptr), quit(false) {
    ep = new Epoll();
}

EventLoop::~EventLoop() {
    delete ep;
}

void EventLoop::loop() {
    printf("server loop on\n");
    while (!quit) {
        std::vector<Channel*> channels;
        channels = ep->poll();
        for (auto it = channels.begin(); it != channels.end(); ++it) {
            (*it)->handle_event();
        }
    }
}

void EventLoop::updateChannel(Channel *ch){
    ep->updateChannel(ch);
}