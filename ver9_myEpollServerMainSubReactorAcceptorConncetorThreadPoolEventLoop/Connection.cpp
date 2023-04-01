#include "Connection.h"
#include "Channel.h"
#include "Socket.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include "util.h"
#include "Buffer.h"
const int READ_BUFFER_SIZE = 1024;

Connection::Connection(EventLoop* _loop, Socket* _socket)
    : loop(_loop)
    , sock(_socket)
    , channel(nullptr)
    , readBuffer(nullptr)
{
    this->channel = new Channel(loop, this->sock->get_fd());
    this->channel->enableRead();
    this->channel->useET();
    std::function<void()> call_back_func_echo = std::bind(&Connection::echo, this, this->sock->get_fd());
    this->channel->setReadCallback(call_back_func_echo);
    this->readBuffer = new Buffer();
}

Connection::~Connection()
{
    delete channel;
    delete sock;
    delete readBuffer;
}

void Connection::echo(int sockfd_client)
{
    char buf[1024];

    while (true) { // 由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕


        bzero(&buf, sizeof(buf));
        ssize_t read_bytes_count = read(sockfd_client, buf, sizeof(buf));
        printf("read_bytes_count: %d, errno: %ld\n", read_bytes_count, errno);

        if (read_bytes_count > 0) {
            readBuffer->append(buf, read_bytes_count);
        } else if (read_bytes_count == -1 && errno == EINTR) { // 客户端正常中断、继续读取?
            printf("由于信号中断，没读到任何数据 client fd:%d closed, continue reading other\n", sockfd_client);
            continue;
        } else if (read_bytes_count == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) { // 非阻塞IO，这个条件表示数据全部读取完毕
            printf("message from client fd %d: %s\n", sockfd_client, readBuffer->c_str());
            printf("finish reading all from fd:%d\n", sockfd_client);
            send(sockfd_client);
            readBuffer->clear();
            break;
        } else if (read_bytes_count == 0) { // EOF，客户端断开连接
            printf("EOF, client fd %d disconnected\n", sockfd_client);
            // close(sockfd_client); // 关闭socket会自动将文件描述符从epoll树上移除
            deleteConnectionCallback(sockfd_client);   
            break;
        } else {
            printf("Connection reset by peer\n");
            deleteConnectionCallback(sockfd_client);         
            break;
        }
    }
}

void Connection::setDeleteConnectionCallback(std::function<void(int)> _call_back_func)
{
    this->deleteConnectionCallback = _call_back_func;
}

void Connection::send(int sockfd){
    char buf[readBuffer->size()];
    strcpy(buf, readBuffer->c_str());
    int data_size = readBuffer->size(); 
    int data_left = data_size; 
    while (data_left > 0) 
    { 
        ssize_t bytes_write = write(sockfd, buf + data_size - data_left, data_left); 
        if (bytes_write == -1 && errno == EAGAIN) { 
            break;
        }
        data_left -= bytes_write; 
    }
}