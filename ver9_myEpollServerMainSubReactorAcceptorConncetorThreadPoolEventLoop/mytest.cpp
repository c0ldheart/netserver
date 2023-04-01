#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sstream>
const char* SERVER_ADDRESS = "127.0.0.1";
const int SERVER_PORT = 1888;
const int THREAD_NUM = 100;
const int MESSAGE_NUM = 10;

void one_client(int id)
{
    std::random_device e;
    std::uniform_real_distribution<double> u(0,3.5);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket failed");
        return;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_address.sin_port = htons(SERVER_PORT);

    if (connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("connect failed");
        close(sockfd);
        return;
    }

    for (int i = 0; i < MESSAGE_NUM; ++i) {
        std::stringstream ss;
        ss << u(e) << " from" << id;

        std::string message = ss.str();

        if (send(sockfd, message.c_str(), message.length(), 0) < 0) {
            perror("send failed");
            close(sockfd);
            return;
        }

        std::cout << "Thread " << id << " sent message: " << message << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    close(sockfd);
}

int main()
{
    std::vector<std::thread> threads;
    for (int i = 0; i < THREAD_NUM; ++i) {
        threads.emplace_back(std::thread(one_client, i));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}