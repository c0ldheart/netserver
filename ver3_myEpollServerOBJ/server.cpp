#include "Channel.h"
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"
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
#include <vector>
const int MAX_EVENTS = 1024;
const int READ_BUFFER_SIZE = 1024;
int main()
{
    void handle_read_event(int sockfd);
    Socket* socket_listen = new Socket();
    InetAddress* address_server = new InetAddress("127.0.0.1", 1888);
    socket_listen->bind(address_server);
    socket_listen->listen();

    Epoll* ep = new Epoll();
    socket_listen->setnonblocking();
    ep->add_fd(socket_listen->get_fd(), EPOLLIN | EPOLLET);

    while (true) { // 不断监听epoll上的事件并处理
        std::vector<epoll_event> events = ep->poll();
        int nfds = events.size();
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == socket_listen->get_fd()) {
                InetAddress* address_client = new InetAddress(); // 会发生内存泄露！没有delete
                int sockfd_client = socket_listen->accept(address_client);
                Socket* socket_client = new Socket(sockfd_client);
                socket_client->setnonblocking();
                ep->add_fd(socket_client->get_fd(), EPOLLIN | EPOLLET);
                printf("new client fd %d accepted(establised)! IP: %s, Port: %d\n", sockfd_client, inet_ntoa(address_client->addr.sin_addr), ntohs(address_client->addr.sin_port));
            } else if (events[i].data.fd & EPOLLIN) {
                handle_read_event(events[i].data.fd);
            } else {
                printf("not supported yet\n");
            }
        }
    }
}

void handle_read_event(int sockfd)
{
    char buf[READ_BUFFER_SIZE];
    int read_times_count = 0;
    while (true) { // 由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        read_times_count++;

        bzero(&buf, sizeof(buf));
        ssize_t read_bytes_count = read(sockfd, buf, sizeof(buf));
        if (read_bytes_count > 0) {
            printf("read %d bytes from fd for %d times, message from client fd %d: %s\n", int(read_bytes_count), read_times_count, sockfd, buf);
            write(sockfd, buf, sizeof(buf)); // echo
        } else if (read_bytes_count == -1 && errno == EINTR) { // 客户端正常中断、继续读取?
            printf("client fd:%d closed, continue reading other\n", sockfd);
            continue;
        } else if (read_bytes_count == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) { // 非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading all from fd:%d\n", sockfd);
            break;
        } else if (read_bytes_count == 0) { // EOF，客户端断开连接
            printf("EOF, client fd %d disconnected\n", sockfd);
            close(sockfd); // 关闭socket会自动将文件描述符从epoll树上移除
            break;
        }
    }
}
