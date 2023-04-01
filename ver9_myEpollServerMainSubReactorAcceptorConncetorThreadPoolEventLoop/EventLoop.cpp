#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include <vector>
#include "ThreadPool.h"
#include <thread>
#include <iostream>
EventLoop::EventLoop() : ep(nullptr), quit(false) {
    ep = new Epoll();
    // threadPool = new ThreadPool(10);
}

EventLoop::~EventLoop() {
    delete ep;
}

void EventLoop::loop() {
    std::thread::id thread_id = std::this_thread::get_id();
    std::cout << "thread id: " << thread_id <<"  server loop on " << std::endl;
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

// void EventLoop::addThread(std::function<void()> func){
//     threadPool->add(func);
// }