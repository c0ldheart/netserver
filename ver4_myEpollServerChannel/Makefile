CC = g++
CFLAGS = -Wall


all: server client

server: server.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp
	$(CC) $(CFLAGS) server.cpp util.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp -o server
	@echo "complie server successful!"

client: client.cpp util.cpp
	$(CC) $(CFLAGS) client.cpp util.cpp -o client
	@echo "complie client successful!"

clean:
	rm -f client server *.o
	@echo "Cleanup completed successfully!"