/*
    Basic C HTTP server written for UNIX systems

    Authored by James Croucher
    Licenced under MIT

    Written referencing The Linux Programming Interface, 2010, Michael Kerrisk
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>

#include "readline.h"

#define SRV_PORT 38032
#define SRV_BIND_ADDR "127.0.0.1"

#define RDLINE_MAX 1024

const char *response = "HTTP/1.1 200 OK\n"
                       "Content-Type: text/html\n"
                       "Connection: close\n\n"
                       "<!DOCTYPE HTML>\n"
                       "<HTML>\n"
                       "<BODY>\n"
                       "<b> Hello world! </b>\n"
                       "</BODY>\n"
                       "</HTML>";

// const char *html =

int main(int argc, char *argv[])
{
    // define ipv4 stream socket (TCP)
    // can use SOCK_DGRAM for UDP
    // can use AF_UNIX for local sockets (i.e., for IPC)
    int s_sock = socket(AF_INET, SOCK_STREAM, 0);

    // allow reuse of the socket if tcp is still cleaning it up
    // (aka previous socket stuck in TIME_WAIT from previous run)
    int option = 1;
    setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (s_sock == -1)
    {
        perror("Failed to create listener socket");
        return EXIT_FAILURE;
    }

    // define binding address and port
    struct sockaddr_in in;

    memset(&in, 0, sizeof(struct sockaddr_in));
    in.sin_family = AF_INET;
    in.sin_port = htons(SRV_PORT);
    // inet_pton supports converting both AF_INET and AF_INET6 addresses
    // inet_aton only supports AF_INET and is considered obsolete
    // returns 1 on success, -1 on invalid AF_*, 0 on invalid IP (no bytes written)
    int pton_res = inet_pton(AF_INET, SRV_BIND_ADDR, &in.sin_addr);
    if (pton_res == -1)
    {
        char output[128];
        sprintf(output, "Failed to write IP address %s to sockaddr_in", SRV_BIND_ADDR);
        perror(output);
        return EXIT_FAILURE;
    }
    else if (!pton_res)
    {
        fprintf(stderr, "Failed to write IP address %s to sockaddr_in: IP address not valid", SRV_BIND_ADDR);
        return EXIT_FAILURE;
    };

    // bind socket to address:port
    if (bind(s_sock, (struct sockaddr *)&in, sizeof(in)))
    {
        char output[128];
        sprintf(output, "Failed to bind socket to address %s:%i", SRV_BIND_ADDR, SRV_PORT);
        perror(output);
        return EXIT_FAILURE;
    };

    // listen for incoming on socket
    // second param is backlog of pending connections
    // SUSv3 specifies maximum backlog is defined as SOMAXCONN
    // (use of this value good for portability)
    if (listen(s_sock, SOMAXCONN))
    {
        char output[128];
        sprintf(output, "Failed to listen on socket %s:%i", SRV_BIND_ADDR, SRV_PORT);
        perror(output);
        return EXIT_FAILURE;
    }

    struct sockaddr r_conn_addr;
    socklen_t conn_len;
    int r_conn_fd;

    for (;;)
    {
        conn_len = sizeof(r_conn_addr);
        r_conn_fd = accept(s_sock, &r_conn_addr, &conn_len);

        if (r_conn_fd != -1)
        {
            char address[INET_ADDRSTRLEN];
            // 16-bit int up to 6 digits long
            char port[6];

            // retrieve remote address:port
            getnameinfo(&r_conn_addr, sizeof(r_conn_addr), address, sizeof(address), port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV);

            printf("got sock (fd: %i) (len: %i) (remote: %s:%s)\n", r_conn_fd, conn_len, address, port);

            // read each line from incoming socket as it arrives
            char nextline[RDLINE_MAX];
            for (;;)
            {
                memset(nextline, 0, RDLINE_MAX);
                ssize_t num = read_line(r_conn_fd, nextline, RDLINE_MAX);
                if (num > 0 && nextline[num - 1] == '\n')
                {
                    // trim newline from end
                    nextline[num - 1] = '\0';
                    printf("got line (len: %i): %s\n", num, nextline);
                    fflush(stdout);

                    if (num == 2)
                    {
                        // all lines received, send response
                        if (send(r_conn_fd, response, strlen(response), 0) == -1)
                        {
                            perror("Failed to send HTTP response");
                            break;
                        };

                        /*
                        if (send(r_conn_fd, html, strlen(html), 0) == -1)
                        {
                            perror("Failed to send HTML response");
                            break;
                        };*/

                        break;
                    }
                }
                else if (num == 0)
                {
                    break;
                }
                else
                {
                    // num == -1
                    perror("Failed to read line from socket");
                    break;
                }
            }

            printf("Shutting down client socket\n");
            if (shutdown(r_conn_fd, SHUT_RDWR))
            {
                perror("Failed to shut down client socket");
                goto exit_error;
            };
        }
        else
        {
            perror("Failed to establish connection with remote socket");
            goto exit_error;
        }
    }

/// Graceful exit of process with EXIT_SUCCESS exit code
exit:
    shutdown(s_sock, SHUT_RDWR);
    return EXIT_SUCCESS;

/// Graceful exit of process with EXIT_FAILURE exit code
exit_error:
    shutdown(s_sock, SHUT_RDWR);
    return EXIT_FAILURE;
}