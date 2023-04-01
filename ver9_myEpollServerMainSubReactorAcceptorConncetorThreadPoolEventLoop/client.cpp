#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <cstring>
#include "util.h"
#include <unistd.h>
#include "Buffer.h"
#include <random>
#include <ctime>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
int main(int argc, char* argv[])
{

    const char* SERVER_ADDRESS = "127.0.0.1";
    int SERVER_PORT = 1888;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_address.sin_port = htons(SERVER_PORT);

    errif(connect(sockfd, (sockaddr*)&server_address, sizeof(server_address)) == -1, "connect error");
    printf("connect success!\n");

    Buffer *sendBuffer = new Buffer();
    Buffer *readBuffer = new Buffer();
    std::random_device e;
    std::uniform_real_distribution<double> u(0,3.5);

    int count = 0;
    // while (true && count++ < 100) {
    //     std::stringstream ss;
    //     std::string s;
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //     ss << u(e);
    //     ss >> s;
    //     sendBuffer->set_buf(s.c_str());
    //     // char buf[1024];
    //     // memset(buf, 0, sizeof(buf));
    //     // scanf("%s", buf);

    //     ssize_t write_bytes = write(sockfd, sendBuffer->c_str(), sendBuffer->size());

    //     errif(write_bytes == -1, "socket already disconnected");

        
    //     int already_read = 0;
    //     char buf[1024];
    //     while(true){
    //         printf("already_read: %d , sendBuffe:%d \n", already_read, sendBuffer->size());

    //         bzero(&buf, sizeof(buf));
    //         ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
    //         if(read_bytes > 0){
    //             readBuffer->append(buf, read_bytes);
    //             already_read += read_bytes;
    //         } else if(read_bytes == 0){         //EOF
    //             printf("server disconnected!\n");
    //             exit(EXIT_SUCCESS);
    //         }
    //         if(already_read >= sendBuffer->size()){
    //             printf("message from server: %s\n", readBuffer->c_str());
    //             break;
    //         } 
    //     }
    //     readBuffer->clear();
    // }
        while(true){
        sendBuffer->getline();
        ssize_t write_bytes = write(sockfd, sendBuffer->c_str(), sendBuffer->size());
        if(write_bytes == -1){
            printf("socket already disconnected, can't write any more!\n");
            break;
        }
        int already_read = 0;
        char buf[1024];    //这个buf大小无所谓
        while(true){
            bzero(&buf, sizeof(buf));
            ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
            if(read_bytes > 0){
                readBuffer->append(buf, read_bytes);
                already_read += read_bytes;
            } else if(read_bytes == 0){         //EOF
                printf("server disconnected!\n");
                exit(EXIT_SUCCESS);
            }
            if(already_read >= sendBuffer->size()){
                printf("message from server: %s\n", readBuffer->c_str());
                break;
            } 
        }
        readBuffer->clear();
    }
}