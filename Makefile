#编译,并试运行
CC      = gcc
CFLAGS  = -Wall -Wextra -O2
TARGETS = server client

all: $(TARGETS)

server: server.c device_protocol.h
	$(CC) $(CFLAGS) server.c -o server

client: client.c device_protocol.h
	$(CC) $(CFLAGS) client.c -o client


clean:
	rm -f $(TARGETS)

.PHONY: all clean