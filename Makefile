CC = gcc

CFLAGS = -g -Wall



EXECUTABLES = web_server web_client

all: ${EXECUTABLES}

web_server: web_server.o connection.o
	${CC} ${CFLAGS} -o web_server web_server.o connection.o

web_client: web_client.o connection.o
	${CC} ${CFLAGS} -o web_client -pthread web_client.o connection.o


clean:
	rm -rf *.o ${EXECUTABLES}
