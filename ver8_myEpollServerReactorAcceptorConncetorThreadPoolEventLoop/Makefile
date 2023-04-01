CC = g++
CFLAGS = -Wall
src = $(wildcard ./*.cpp)

all: server client

server: server.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp
	$(CC) $(CFLAGS) server.cpp  \
	-pthread \
	util.cpp InetAddress.cpp Socket.cpp  \
	Epoll.cpp Channel.cpp EventLoop.cpp Server.cpp Acceptor.cpp Connection.cpp Buffer.cpp ThreadPool.cpp  \
	-o server
	@echo "complie server successful!"

client: client.cpp util.cpp
	$(CC) $(CFLAGS) client.cpp util.cpp -o client
	@echo "complie client successful!"

clean:
	rm -f client server test *.o
	@echo "Cleanup completed successfully!"