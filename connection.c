#include "connection.h"

void err_n_die(const char *fmt, ...) {
    int errno_save;
    va_list ap;

    // any system or library call can errno, so we need to save it now
    errno_save = errno;

    // print out the fmt+args to standard out
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);

    // print our error message is errno was set.
    if (errno_save != 0) {
        fprintf(stdout, "(errno = %d) : %s\n", errno_save, strerror(errno_save));
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    va_end(ap);

    // this is the ..and_die part, Terminate with an error.
    exit(1);
}

ssize_t readline(int fd, void *buf, size_t maxlen) {
    char c;
    char *bufp = buf;
    int n;
    for (n = 0; n < maxlen - 1; n++) {  // leave room at end for '\0'
        int rc;
        if ((rc = read(fd, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            if (n == 1)
                return 0; /* EOF, no data read */
            else
                break; /* EOF, some data was read */
        } else
            return -1; /* error */
    }
    *bufp = '\0';
    return n;
}

int open_server_connection() {
    // Create a socket descriptor
    int listen_fd;
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    // Eliminates "Address already in use" error from bind
    int opt_val = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt_val, sizeof(int)) < 0) {
        return -2;
    }

    // Endpoint for all requests to port on any IP address for this host
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT); /* server port */

    if ((bind(listen_fd, (SA *)&server_addr, sizeof(server_addr))) < 0) {
        return -3;
    }

    if ((listen(listen_fd, 1024)) < 0) {
        // accepts connection requests
        return -4;
    }

    return listen_fd;
}

// request error
void request_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXLINE], body[MAXLINE];

    // Create the body of error message first (have to know its length for header)
    sprintf(body,
            ""
            "<!doctype html>\r\n"
            "<head>\r\n"
            "  <title>OSTEP WebServer Error</title>\r\n"
            "</head>\r\n"
            "<body>\r\n"
            "  <h2>%s: %s</h2>\r\n"
            "  <p>%s: %s</p>\r\n"
            "</body>\r\n"
            "</html>\r\n",
            errnum, shortmsg, longmsg, cause);

    // Write out the header information for this response
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    if (write(fd, buf, strlen(buf)) < 0) {
        err_n_die("write error.");
    }

    sprintf(buf, "Content-Type: text/html\r\n");
    if (write(fd, buf, strlen(buf)) < 0) {
        err_n_die("write error.");
    }

    sprintf(buf, "Content-Length: %lu\r\n\r\n", strlen(body));
    if (write(fd, buf, strlen(buf)) < 0) {
        err_n_die("write error.");
    }

    // Write out the body last
    if (write(fd, body, strlen(body)) < 0) {
        err_n_die("write error.");
    }
}

//
// Reads and discards everything up to an empty text line
//
void request_read_headers(int fd) {
    char buf[MAXLINE];

    if (readline(fd, buf, MAXLINE) < 0) {
        err_n_die("readline error.");
    }
    while (strcmp(buf, "\r\n")) {
        if (readline(fd, buf, MAXLINE) < 0) {
            err_n_die("readline error.");
        }
    }
    return;
}

//
// Return 1 if static, 0 if dynamic content
// Calculates filename (and cgiargs, for dynamic) from uri
//
int request_parse_uri(char *uri, char *filename, char *cgiargs) {
    char *ptr;

    if (!strstr(uri, "cgi")) {
        // static
        strcpy(cgiargs, "");
        sprintf(filename, ".%s", uri);
        if (uri[strlen(uri) - 1] == '/') {
            strcat(filename, "index.html");
        }
        return 1;
    } else {
        // dynamic
        ptr = index(uri, '?');
        if (ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        } else {
            strcpy(cgiargs, "");
        }
        sprintf(filename, ".%s", uri);
        return 0;
    }
}

//
// Fills in the filetype given the filename
//
void request_get_filetype(char *filename, char *filetype) {
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

void request_serve_dynamic(int fd, char *filename, char *cgiargs) {
    char buf[MAXLINE], *argv[] = {NULL};

    // The server does only a little bit of the header.
    // The CGI script has to finish writing out the header.
    sprintf(buf,
            ""
            "HTTP/1.0 200 OK\r\n"
            "Server: IIITH WebServer\r\n");

    if (write(fd, buf, strlen(buf)) < 0) {
        err_n_die("write error.");
    }

    pid_t pid = fork();
    if (pid < 0) {
        err_n_die("fork error.");
    }

    if (pid == 0) {                                    // child
        if (setenv("QUERY_STRING", cgiargs, 1) < 0) {  // args to cgi go here
            err_n_die("setenv error.");
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {  // make cgi writes go to socket (not screen)
            err_n_die("dup2 error.");
        }
        extern char **environ;  // defined by libc
        if (execve(filename, argv, environ) < 0) {
            err_n_die("execve error.");
        }
    } else {
        if (wait(NULL) < 0) {
            err_n_die("wait error.");
        }
    }
}

void request_serve_static(int fd, char *filename, int filesize) {
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXLINE];

    request_get_filetype(filename, filetype);
    if ((srcfd = open(filename, O_RDONLY, 0)) < 0) {
        err_n_die("open error.");
    }

    // Rather than call read() to read the file into memory,
    // which would require that we allocate a buffer, we memory-map the file
    if ((srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0)) < 0) {
        err_n_die("mmap error.");
    }

    if (close(srcfd) < 0) {
        err_n_die("close error.");
    }

    // put together response
    sprintf(buf,
            ""
            "HTTP/1.0 200 OK\r\n"
            "Server: IIITH WebServer\r\n"
            "Content-Length: %d\r\n"
            "Content-Type: %s\r\n\r\n",
            filesize, filetype);

    if (write(fd, buf, strlen(buf)) < 0) {
        err_n_die("write error.");
    }
    //  Writes out to the client socket the memory-mapped file
    if (write(fd, srcp, filesize) < 0) {
        err_n_die("write error.");
    }
    if (munmap(srcp, filesize) < 0) {
        err_n_die("munmap error.");
    }
}
/* Function to read the data present at the given line number 
 * and send the data as response */
void read_line_file(int fd , int line_number){
    // semaphore to be added here.  
    FILE *file = fopen("data.txt" ,"r") ; 
    if (file ==NULL){
        err_n_die("Unable to open the file"); 
    }
    char buf[MAXLINE], header[MAXLINE]; 

    int line =1 , found=0 ;
    
    while(fgets(buf , MAXLINE , file )){
        if (line == line_number){
            found =1 ;
            break ;
        }
        line++ ; 
    }
    if(found ){
        sprintf(header,
            ""
            "HTTP/1.0 200 OK\r\n"
            "Server: IIITH WebServer\r\n"
            "Content-Length: %d\r\n"
            "Content-Type: \r\n\r\n",
            strlen(buf));
        if (write(fd, header , strlen(header))< 0){
            err_n_die("write error"); 
        }
        if (write(fd, buf , strlen(buf))< 0){
            err_n_die("write error") ; 
        }
    }else {
        err_n_die("Unable to read this line"); 
    }
}

// handle a request
void handle_request(int fd) {
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];

    if (readline(fd, buf, MAXLINE) < 0) {
        err_n_die("readline error.");
    }

    sscanf(buf, "%s %s %s", method, uri, version);
    printf("method:%s uri:%s version:%s\n", method, uri, version);
    fflush(stdout) ; 
    if (strcasecmp(method, "GET")) {
        request_error(fd, method, "501", "Not Implemented", "server does not implement this method");
        return;
    }

    // Start by reading headers.
    request_read_headers(fd);

    // Check if the requested file is present or not.
    struct stat sbuf;
    char filename[MAXLINE], cgiargs[MAXLINE];
    int is_static = request_parse_uri(uri, filename, cgiargs);

    if (stat(filename, &sbuf) < 0) {
        request_error(fd, filename, "404", "Not found", "server could not find this file");
        return;
    }

    if (is_static) {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
            request_error(fd, filename, "403", "Forbidden", "server could not read this file");
            return;
        }
        request_serve_static(fd, filename, sbuf.st_size);
    } else {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
            request_error(fd, filename, "403", "Forbidden", "server could not run this CGI program");
            return;
        }
        request_serve_dynamic(fd, filename, cgiargs);
    }
}

