#include "Socket.h"
#include "InetAddress.h"
#include "util.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cstdio>
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

Socket::~Socket()
{
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

void Socket::bind(InetAddress* address)
{
    struct sockaddr_in addr = address->get_addr();
    errif(::bind(fd, (sockaddr*)&addr, sizeof(addr)) == -1, "bind error");
}

void Socket::listen()
{
    errif(::listen(fd, SOMAXCONN) == -1, "listen error");
}

int Socket::accept(InetAddress* _addr)
{
    int clnt_sockfd = -1;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addr_len = sizeof(addr);
    if(fcntl(fd, F_GETFL) & O_NONBLOCK){
        while(true){
            clnt_sockfd = ::accept(fd, (sockaddr*)&addr, &addr_len);
            if(clnt_sockfd == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){
                printf("no connection yet\n");
                continue;
            } else if(clnt_sockfd == -1){
                errif(true, "socket accept error");
            } else{
                break;
            }
        }
    }
    else{
        clnt_sockfd = ::accept(fd, (sockaddr*)&addr, &addr_len);
        errif(clnt_sockfd == -1, "socket accept error");
    }
    _addr->set_inet_addr(addr, addr_len);
    return clnt_sockfd;
    // int sockfd_client = ::accept(fd, (sockaddr*)&addr, &addr_len);
    // errif(sockfd_client == -1, "accept error");
    // return sockfd_client;
}

void Socket::setnonblocking()
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

void Socket::connect(InetAddress* _addr)
{
    // struct sockaddr_in addr = _addr->get_addr();
    // errif(::connect(fd, (sockaddr*)&addr, sizeof(addr)) == -1, "socket connect error");
    
    // for client socket
    struct sockaddr_in addr = _addr->get_addr();
    if (fcntl(fd, F_GETFL) & O_NONBLOCK) {
        while (true) {
            int ret = ::connect(fd, (sockaddr*)&addr, sizeof(addr));
            if (ret == 0) {
                break;
            } else if (ret == -1 && (errno == EINPROGRESS)) {
                continue;
                /* 连接非阻塞式sockfd建议的做法：
                    The socket is nonblocking and the connection cannot be
                  completed immediately.  (UNIX domain sockets failed with
                  EAGAIN instead.)  It is possible to select(2) or poll(2)
                  for completion by selecting the socket for writing.  After
                  select(2) indicates writability, use getsockopt(2) to read
                  the SO_ERROR option at level SOL_SOCKET to determine
                  whether connect() completed successfully (SO_ERROR is
                  zero) or unsuccessfully (SO_ERROR is one of the usual
                  error codes listed here, explaining the reason for the
                  failure).
                  这里为了简单、不断连接直到连接完成，相当于阻塞式
                */
            } else if (ret == -1) {
                errif(true, "socket connect error");
            }
        }
    } else {
        errif(::connect(fd, (sockaddr*)&addr, sizeof(addr)) == -1, "socket connect error");
    }
}

int Socket::get_fd()
{
    return fd;
}