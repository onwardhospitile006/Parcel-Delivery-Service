#ifndef NETWORK_H
#define NETWORK_H

#define SERVER_PORT 8080
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 10
#define END_MARKER "END\n"

// Send a complete message (newline-terminated) over a socket
int send_msg(int sockfd, const char *msg);

// Receive a single newline-terminated line from a socket
int recv_msg(int sockfd, char *buf, int size);

// Receive multiple lines until END marker
int recv_multiline(int sockfd, char *buf, int size);

#endif
