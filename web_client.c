#include "connection.h"
#include <pthread.h>

#define MAXBUF (8192)

struct arg_struct
{
    char *host;
    int port;
    char *filename;
};

void client_send(int fd, char *filename)
{
    char buf[MAXBUF];
    char hostname[MAXBUF];

    if (gethostname(hostname, MAXBUF) < 0)
        err_n_die("gethostname error");

    /* Form and send the HTTP request */
    sprintf(buf, "GET %s HTTP/1.1\n", filename);
    sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
    if (write(fd, buf, strlen(buf)) < 0)
        err_n_die("write error");
}
//
// Read the HTTP response and print it out
//
void client_print(int fd)
{
    char buf[MAXBUF];
    int n;
    // Read and display the HTTP Header
    n = readline(fd, buf, MAXBUF);
    if (n == -1)
        err_n_die("readline error");
    while (strcmp(buf, "\r\n") && (n > 0))
    {
        printf("Header: %s", buf);
        n = readline(fd, buf, MAXBUF);
        if (n == -1)
            err_n_die("readline error");
    }

    // Read and display the HTTP Body
    n = readline(fd, buf, MAXBUF);
    if (n == -1)
        err_n_die("readline error");
    while (n > 0)
    {
        printf("%s", buf);
        n = readline(fd, buf, MAXBUF);
        if (n == -1)
            err_n_die("readline error");
    }
}

void *rountine(void *arguments)
{
    struct arg_struct *args = (struct arg_struct *)arguments;
    int clientfd;
    /* Open a single connection to the specified host and port */
    clientfd = open_client_connection(args->host, args->port);
    if (clientfd == -1)
        err_n_die("open_client_connection error");

    client_send(clientfd, args->filename);
    client_print(clientfd);

    if (close(clientfd) < 0)
        err_n_die("close error");

    return NULL;
}

int main(int argc, char *argv[])
{
    char *host, *filename;
    int port;
    int clientfd;

    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[2]);
    filename = argv[3];
    host = argv[1];

    struct arg_struct args;
    args.port = port;
    args.host = host;
    args.filename = filename;

    int THREAD_POOL_SIZE = 100;
    pthread_t t[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        pthread_create(&t[i], NULL, &rountine, (void *)&args);
    }

    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        pthread_join(t[i], NULL);
    }

    exit(0);
}
