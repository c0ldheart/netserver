#include "util.h"
#include <arpa/inet.h> // address结构体, 类型转换
#include <cstdio>
#include <cstring> // bzero
#include <errno.h> // 错误处理
#include <fcntl.h> // 设置非阻塞socket
#include <sys/epoll.h>
#include <sys/socket.h> // 建立socket文件描述符
#include <sys/types.h> // scoklen_t
#include <unistd.h>
const int MAX_EVENTS = 1024;
const int READ_BUFFER_SIZE = 1024;
int main()
{
    int sockfd_listen = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd_listen == -1, "socket listen create error");

    struct sockaddr_in server_address_listen;
    bzero(&server_address_listen, sizeof(server_address_listen));
    server_address_listen.sin_family = AF_INET;
    server_address_listen.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address_listen.sin_port = htons(1889);

    errif(bind(sockfd_listen, (struct sockaddr*)&server_address_listen, sizeof(server_address_listen)) == -1, "socket bind error");

    errif(listen(sockfd_listen, SOMAXCONN) == -1, "socket listen error");

    int epfd = epoll_create1(0); // 建立epoll fd

    struct epoll_event events[MAX_EVENTS], ev;
    bzero(&events, sizeof(events));
    bzero(&ev, sizeof(ev));
    ev.events = EPOLLIN; // ET模式
    ev.data.fd = sockfd_listen; // 该IO口为服务器socket fd
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd_listen, &ev); // 将服务器listen socket fd添加到epoll

    while (true) { // 不断监听epoll上的事件并处理
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1); // 有nfds个fd发生事件
        for (int i = 0; i < nfds; ++i) { // 处理这nfds个事件
            if (events[i].data.fd == sockfd_listen) { // 当前处理的事件是listenfd 表示有新客户端连接
                struct sockaddr_in client_address;
                bzero(&client_address, sizeof(client_address));
                socklen_t client_address_len = sizeof(client_address);

                int sockfd_client = accept(sockfd_listen, (struct sockaddr*)&client_address, &client_address_len);
                errif(sockfd_client == -1, "socket accept error");
                printf("new client fd %d accepted(establised)! IP: %s, Port: %d\n", sockfd_client, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

                // 这个新连接加到epoll: 1. 建立event类型，调用epoll_ctl
                bzero(&ev, sizeof(ev));
                fcntl(sockfd_client, F_SETFL, fcntl(sockfd_client, F_GETFL) | O_NONBLOCK); // ET模式需要非阻塞socket
                ev.events = EPOLLIN | EPOLLET; // 使用ET模式
                ev.data.fd = sockfd_client;
                epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd_client, &ev); // 添加到epoll
            } else if (events[i].events & EPOLLIN) { // 检测到可读事件
                char buf[READ_BUFFER_SIZE];
                int read_times_count = 0;
                while (true) { // 由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
                    read_times_count++;

                    bzero(&buf, sizeof(buf));
                    ssize_t read_bytes_count = read(events[i].data.fd, buf, sizeof(buf));
                    if (read_bytes_count > 0) {
                        printf("read %d bytes from fd for %d times, message from client fd %d: %s\n", int(read_bytes_count), read_times_count, events[i].data.fd, buf);
                        write(events[i].data.fd, buf, sizeof(buf)); // echo
                    } else if (read_bytes_count == -1 && errno == EINTR) { // 客户端正常中断、继续读取?
                        printf("client fd:%d closed, continue reading other\n", events[i].data.fd);
                        continue;
                    } else if (read_bytes_count == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) { // 非阻塞IO，这个条件表示数据全部读取完毕
                        printf("finish reading all from fd:%d\n", events[i].data.fd);
                        break;
                    } else if (read_bytes_count == 0) { // EOF，客户端断开连接
                        printf("EOF, client fd %d disconnected\n", events[i].data.fd);
                        close(events[i].data.fd); // 关闭socket会自动将文件描述符从epoll树上移除
                        break;
                    }
                }
            } else {
                printf("not supported yet\n");
            }
        }
    }
    return 0;
}
