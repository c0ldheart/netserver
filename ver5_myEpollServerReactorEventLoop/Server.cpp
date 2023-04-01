#include "EventLoop.h"
#include "Server.h"

int main() {
    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop);  // 构造函数中初始化最初的监听socket到loop(ep)
    loop->loop();
    return 0;
}