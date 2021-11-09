CC = gcc

CFLAGS = -g -Wall

EXECUTABLES = web_server

all: ${EXECUTABLES}

web_server: web_server.o connection.o
	${CC} ${CFLAGS} -o web_server web_server.o connection.o

clean:
	rm -rf *.o ${EXECUTABLES}
