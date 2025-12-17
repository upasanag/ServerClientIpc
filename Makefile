CC = g++
CFLAGS = -std=c++17 -pthread
TARGETS = server client

all: $(TARGETS)

server: server.cpp config.h
	$(CC) $(CFLAGS) server.cpp -o server

client: client.cpp config.h
	$(CC) $(CFLAGS) client.cpp -o client

clean:
	rm -f $(TARGETS) /tmp/log.tmp