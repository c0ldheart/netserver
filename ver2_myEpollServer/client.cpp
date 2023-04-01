#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <cstring>
#include "util.h"
#include <unistd.h>


int main(int argc, char* argv[])
{

    const char* SERVER_ADDRESS = "127.0.0.1";
    int SERVER_PORT = 1889;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_address.sin_port = htons(SERVER_PORT);

    errif(connect(sockfd, (sockaddr*)&server_address, sizeof(server_address)) == -1, "connect error");
    printf("connect success!\n");
    while (true) {
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        scanf("%s", buf);

        ssize_t write_bytes = write(sockfd, buf, sizeof(buf));

        errif(write_bytes == -1, "socket already disconnected");


        memset(buf, 0, sizeof(buf));
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("message from server: %s\n", buf);
        } else if (read_bytes == 0) {
            printf("server socket closed");
            break;
        } else if (read_bytes == -1) {
            close(sockfd);
            errif(true, "socket read error");
        }
    }
}