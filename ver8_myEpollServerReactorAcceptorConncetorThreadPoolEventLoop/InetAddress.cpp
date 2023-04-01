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

void InetAddress::set_Inet_addr(sockaddr_in _addr, socklen_t _addr_len){
    addr = _addr;
    addr_len = _addr_len;
}

sockaddr_in InetAddress::get_addr(){
    return addr;
}
socklen_t InetAddress::get_addr_len(){
    return addr_len;
}