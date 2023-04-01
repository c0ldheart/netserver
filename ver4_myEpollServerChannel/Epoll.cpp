#include "Epoll.h"
#include "Channel.h"
#include "util.h"
#include <cstring>
#include <unistd.h>
const int MAX_EVENTS = 1024;
Epoll::Epoll()
    : epfd(-1)
    , events(nullptr)
{
    this->epfd = epoll_create1(0);
    errif(this->epfd < 0, "create epoll failed");

    this->events = new epoll_event[MAX_EVENTS];
    memset(this->events, 0, sizeof(epoll_event));
}

Epoll::~Epoll()
{
    if (epfd != -1) {
        close(epfd);
        epfd = -1;
    }
    delete[] events;
}

void Epoll::add_fd(int fd, uint32_t option)
{
    struct epoll_event tmp_ev;
    memset(&tmp_ev, 0, sizeof(tmp_ev));
    tmp_ev.data.fd = fd;
    tmp_ev.events = option;

    errif(epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &tmp_ev) == -1, "epoll add event error");
}

std::vector<Channel*> Epoll::poll(int timeout)
{
    std::vector<Channel*> active_events;
    int nfds = epoll_wait(this->epfd, this->events, MAX_EVENTS, timeout);
    errif(nfds == -1, "epoll wait error");
    for (int i = 0; i < nfds; ++i) {
        Channel* ch = (Channel*)events[i].data.ptr;
        ch->setRevents(events[i].events);
        active_events.push_back(ch);
    }
    return active_events;
}

void Epoll::updateChannel(Channel* channel)
{
    int fd = channel->getFd();
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();
    if (!channel->getInEpoll()) {
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
        channel->setInEpoll();
        // debug("Epoll: add Channel to epoll tree success, the Channel's fd is: ", fd);
    } else {
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
        // debug("Epoll: modify Channel in epoll tree success, the Channel's fd is: ", fd);
    }
}