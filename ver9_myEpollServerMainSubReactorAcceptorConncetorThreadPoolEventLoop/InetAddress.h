#pragma once
#include <arpa/inet.h>

class InetAddress {
public:
    struct sockaddr_in addr;
    socklen_t addr_len;
    InetAddress();
    InetAddress(const char* ip, uint16_t port);
    ~InetAddress();

    void set_inet_addr(sockaddr_in _addr, socklen_t _addr_len);
    sockaddr_in get_addr();
    socklen_t get_addr_len();

    char* get_ip();
    uint16_t get_port();
};
