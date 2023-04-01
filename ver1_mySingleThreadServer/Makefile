CC = g++
CFLAGS = -Wall


all: server client


server: server.cpp util.h
	$(CC) $(CFLAGS) server.cpp util.cpp -o server


client: client.cpp util.h
	$(CC) $(CFLAGS) client.cpp util.cpp -o client

clean:
	rm -f client server *.o
	@echo "Cleanup completed successfully!"