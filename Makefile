CC = gcc

CFLAGS = -g -Wall

EXECUTABLES = web_server

all: ${EXECUTABLES}

web_server: web_server.o connection.o llist.o
	${CC} ${CFLAGS} -o web_server web_server.o connection.o llist.o

clean:
	rm -rf *.o ${EXECUTABLES} web_server.o connection.o llist.o
