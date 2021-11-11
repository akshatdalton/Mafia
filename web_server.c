#include "connection.h"
#include "llist.h"
#include <strings.h>
#include <pthread.h>

#define THREAD_POOL_SIZE 20

pthread_t thread_pool[THREAD_POOL_SIZE];

//handle multiple threads
void *thread_function(void *arg){
    while (1){
        int *pclient = dequeue();
        if (pclient != NULL)
            handle_request(*pclient);
    }
}

int main(int argc, char *argv[]) {
    int listen_fd, conn_fd;

    if ((listen_fd = open_server_connection()) < 0) {
        err_n_die("open_server_connection error.");
    }

    for (int i=0;i<THREAD_POOL_SIZE;i++){
        pthread_create(&thread_pool[i],NULL,thread_function,NULL);
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        //put connection decsription ina linked list

        // accept blocks untill an incoming connection arrives
        // it returns a "file descriptor" to the connection.
        printf("waiting for a connection on port %d\n", SERVER_PORT);
        fflush(stdout);
        if ((conn_fd = accept(listen_fd, (SA *) &client_addr, (SA *) &addr_len)) < 0) {
            err_n_die("accept error.");
        }
        printf("Connected!\n");
        int *pclient = malloc(sizeof(int));
        *pclient = conn_fd;
        enqueue(pclient);
        close(conn_fd);
    }
}
