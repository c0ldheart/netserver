#include "InetAddress.h"
#include <cstring>

InetAddress::InetAddress()
    : addr_len(sizeof(addr))
{
    memset(&addr, 0, sizeof(addr));
}

InetAddress::InetAddress(const char* ip_, uint16_t port_)
    : addr_len(sizeof(addr))
{
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_);
    addr.sin_port = htons(port_);
}

InetAddress::~InetAddress() {

}