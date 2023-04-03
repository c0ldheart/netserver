#include <Connection.h>
#include <Socket.h>
#include <iostream>

int main() {
    setbuf(stdout,NULL);
  Socket *sock = new Socket();
  sock->Connect("192.168.173.23", 1234);
  std::cout<<"Connect success!" << std::endl;
  Connection *conn = new Connection(nullptr, sock);

  while (true) {
//    conn->GetlineSendBuffer();
    conn->SetSendBuffer("GET /?ping=<ping>&hello=<hello> HTTP/1.1\r\n"
                        "Host: 192.168.173.23:1234\r\n"
                        "Accept: */*\r\n"
                        "User-Agent: Apifox/1.0.0 (https://www.apifox.cn)");
    if (conn->GetWriteBufferSize() == 0) {
      std::cout << "input can't be empty, please try again." << std::endl;
      continue;
    }
    conn->Write();
    if (conn->GetState() == Connection::State::Closed) {
      std::cout << "Server has been Closed" << std::endl;
      conn->Close();
      break;
    }
    conn->Read();
    if (conn->GetState() == Connection::State::Closed) {
      std::cout << "Server Closed" << std::endl;
      conn->Close();
      break;
    }
    std::cout << "Message from server: " << conn->ReadBuffer() << std::endl;
  }

  delete conn;
  return 0;
}
