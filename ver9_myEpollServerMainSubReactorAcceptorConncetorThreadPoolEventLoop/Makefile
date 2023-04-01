CC = g++
CFLAGS = -Wall
src = $(wildcard ./*.cpp)

all: server client test

server: server.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp
	$(CC) $(CFLAGS) server.cpp  \
	-pthread \
	util.cpp InetAddress.cpp Socket.cpp  \
	Epoll.cpp Channel.cpp EventLoop.cpp Server.cpp Acceptor.cpp Connection.cpp Buffer.cpp ThreadPool.cpp  \
	-o server
	@echo "complie server successful!"

client: client.cpp util.cpp
	$(CC) $(CFLAGS) client.cpp util.cpp Buffer.cpp -o client
	@echo "complie client successful!"

test:
	g++ mytest.cpp ThreadPool.cpp util.cpp Buffer.cpp Socket.cpp InetAddress.cpp -pthread -o test
clean:
	rm -f client server test *.o
	@echo "Cleanup completed successfully!"