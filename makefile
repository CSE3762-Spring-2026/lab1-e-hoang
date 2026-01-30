# Makefile for client and server

CC = gcc
OBJCS = client1.c

CFLAGS =  -g -Wall
# setup for system
nLIBS =

all: server client1 

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client1: $(OBJCS)
	$(CC) $(CFLAGS) -o $@ $(OBJCS) $(LIBS)

clean:
	rm client1
