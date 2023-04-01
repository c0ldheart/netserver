#include "util.h"
#include <arpa/inet.h> // address结构体, 类型转换
#include <cstdio>
#include <cstring> // bzero
#include <sys/socket.h> // 建立socket文件描述符
#include <sys/types.h> // scoklen_t
#include <unistd.h>
int main()
{
    int sockfd_listen = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd_listen == -1, "socket listen create error");

    struct sockaddr_in server_address_listen;
    bzero(&server_address_listen, sizeof(server_address_listen));
    server_address_listen.sin_family = AF_INET;
    server_address_listen.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address_listen.sin_port = htons(1888);

    errif(bind(sockfd_listen, (struct sockaddr*)&server_address_listen, sizeof(server_address_listen)) == -1, "socket bind error");

    errif(listen(sockfd_listen, SOMAXCONN) == -1, "socket listen error");

    // 建立连接


    struct sockaddr_in client_address;

    socklen_t client_address_len = sizeof(client_address);
    bzero(&client_address, client_address_len);

    int sockfd_accept = accept(sockfd_listen, (struct sockaddr*)&client_address, &client_address_len); //  阻塞。等待accept, 并更新client_address地址；写入客户端socket长度。
    errif(sockfd_accept == -1, "accepte error");

    printf("new client fd %d accepted(establised)! IP: %s, Port: %d\n", sockfd_accept, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
    while (true) {
        char buf[1024];
        memset(buf, 0, sizeof(buf));

        ssize_t read_bytes = read(sockfd_accept, buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("message from client: %s\n", buf);
            ssize_t write_bytes = write(sockfd_accept, buf, sizeof(buf));
            errif(write_bytes == -1, "socket already disconnected");

        } else if (read_bytes == 0) {
            printf("client fd: %d disconnected.\n", sockfd_accept);
            close(sockfd_accept);
            break;
        } else if (read_bytes == -1) {
            close(sockfd_accept);
            errif(true, "socket read error");
        }
    }

    return 0;
}
