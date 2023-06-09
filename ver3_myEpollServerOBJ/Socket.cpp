#include "Socket.h"
#include "InetAddress.h"
#include "util.h"
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
Socket::Socket()
    : fd(-1)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd == -1, "socket create error");
}

Socket::Socket(int fd_)
    : fd(fd_)
{
    errif(fd == -1, "socket create error");
}


Socket::~Socket(){
    if(fd != -1){
        close(fd);
        fd = -1;
    }
}

void Socket::bind(InetAddress* address) {
    errif(::bind(fd, (sockaddr*)&address->addr, address->addr_len) == -1, "bind error");
}

void Socket::listen() {
    errif(::listen(fd, SOMAXCONN) == -1, "listen error");
}

int Socket::accept(InetAddress* addr) {
    int sockfd_client = ::accept(fd, (sockaddr*)&addr->addr, &addr->addr_len);
    errif(sockfd_client == -1, "accept error");
    return sockfd_client;
}

void Socket::setnonblocking(){
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int Socket::get_fd() {
    return fd;
}