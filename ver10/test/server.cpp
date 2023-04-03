#include "Server.h"
#include <iostream>
#include <sstream>
#include "Buffer.h"
#include "Connection.h"
#include "EventLoop.h"
#include "Socket.h"

void handle_http_request(Connection *conn) {
    conn->Read();
    if (conn->GetState() == Connection::State::Closed) {
        conn->Close();
        return;
    }

    std::string request = conn->GetReadBuffer()->ToStr();

    // 解析请求
    std::istringstream request_stream(request);
    std::string method, path, version;
    request_stream >> method >> path >> version;

    // 检查方法是否为GET
    if (method != "GET") {
        conn->SetSendBuffer("HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n");
        conn->Write();
        return;
    }

    // 解析查询参数
    std::map<std::string, std::string> query_params;
    std::size_t query_pos = path.find('?');
    if (query_pos != std::string::npos) {
        std::string query_string = path.substr(query_pos + 1);
        std::istringstream query_stream(query_string);
        std::string key_value_pair;
        while (std::getline(query_stream, key_value_pair, '&')) {
            std::size_t equals_pos = key_value_pair.find('=');
            if (equals_pos != std::string::npos) {
                std::string key = key_value_pair.substr(0, equals_pos);
                std::string value = key_value_pair.substr(equals_pos + 1);
                query_params[key] = value;
            }
        }
        path = path.substr(0, query_pos);
    }

    // 构造响应
    std::ostringstream response_stream;
    std::cout << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
    std::cout << "Method: " << method << "\r\n";
    std::cout << "Path: " << path << "\r\n";
    for (auto &pair: query_params) {
        std::cout << "Query param: " << pair.first << "=" << pair.second << "\r\n";
    }


    conn->SetSendBuffer("HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nHi!");
    conn->Write();
}

int main() {
    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop);
//   server->OnConnect([](Connection *conn) {
//     conn->Read();
//     if (conn->GetState() == Connection::State::Closed) {
//       conn->Close();
//       return;
//     }
//     std::cout << "Message from client " << conn->GetSocket()->GetFd() << ": " << conn->ReadBuffer() << std::endl;
//     conn->SetSendBuffer(conn->ReadBuffer());
//     conn->Write();
//   });
    server->OnConnect([](Connection *conn) {
        handle_http_request(conn);
    });

    loop->Loop();
    delete server;
    delete loop;
    return 0;
}
