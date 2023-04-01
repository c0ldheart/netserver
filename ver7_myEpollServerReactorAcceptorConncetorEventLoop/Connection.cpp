#include "Connection.h"
#include "Channel.h"
#include "Socket.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include "util.h"
const int READ_BUFFER_SIZE = 1024;

Connection::Connection(EventLoop* _loop, Socket* _socket)
    : loop(_loop)
    , sock(_socket)
    , channel(nullptr)
{
    this->channel = new Channel(loop, this->sock->get_fd());
    std::function<void()> call_back_func_echo = std::bind(&Connection::echo, this, this->sock->get_fd());
    this->channel->set_callback(call_back_func_echo);
    this->channel->enableReading();
    this->readBuffer = new Buffer();
}

Connection::~Connection()
{
    delete channel;
    delete sock;
}

void Connection::echo(int sockfd_client)
{
    char buf[READ_BUFFER_SIZE];
    int read_times_count = 0;
    while (true) { // 由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        read_times_count++;

        memset(&buf, 0, sizeof(buf));
        ssize_t read_bytes_count = read(sockfd_client, buf, sizeof(buf));
        if (read_bytes_count > 0) {
            readBuffer->append(buf, read_bytes_count);
        } else if (read_bytes_count == -1 && errno == EINTR) { // 客户端正常中断、继续读取?
            printf("client fd:%d closed, continue reading other\n", sockfd_client);
            continue;
        } else if (read_bytes_count == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) { // 非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading all from fd:%d\n", sockfd_client);
            printf("message from client fd %d: %s\n", sockfd_client, readBuffer->c_str());
            errif(write(sockfd_client, readBuffer->c_str(), readBuffer->size()) == -1, "socket echo write error");
            readBuffer->clear();
            break;
        } else if (read_bytes_count == 0) { // EOF，客户端断开连接
            printf("EOF, client fd %d disconnected\n", sockfd_client);
            close(sockfd_client); // 关闭socket会自动将文件描述符从epoll树上移除
            break;
        }
    }
}

void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> _call_back_func)
{
    this->deleteConnectionCallback = _call_back_func;
}