#include "connection.h"
#include <strings.h>
#include "llist.h"

int main(int argc, char *argv[]) {
    int listen_fd, conn_fd;

    if ((listen_fd = open_server_connection()) < 0) {
        err_n_die("open_server_connection error.");
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        // accept blocks untill an incoming connection arrives
        // it returns a "file descriptor" to the connection.
        printf("waiting for a connection on port %d\n", SERVER_PORT);
        fflush(stdout);
        if ((conn_fd = accept(listen_fd, (SA *) &client_addr, (SA *) &addr_len)) < 0) {
            err_n_die("accept error.");
        }

        handle_request(conn_fd);

        close(conn_fd);
    }
}
