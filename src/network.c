#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "../include/network.h"

int send_msg(int sockfd, const char *msg) {
    int len = strlen(msg);
    int total = 0;

    while (total < len) {
        int n = write(sockfd, msg + total, len - total);
        if (n <= 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        total += n;
    }
    return total;
}

int recv_msg(int sockfd, char *buf, int size) {
    int i = 0;
    char c;

    while (i < size - 1) {
        int n = read(sockfd, &c, 1);
        if (n <= 0) {
            if (n == 0) break;      // connection closed
            if (errno == EINTR) continue;
            return -1;
        }
        buf[i++] = c;
        if (c == '\n') break;
    }

    buf[i] = '\0';
    return i;
}

int recv_multiline(int sockfd, char *buf, int size) {
    int total = 0;
    char line[BUFFER_SIZE];

    buf[0] = '\0';

    while (total < size - 1) {
        int n = recv_msg(sockfd, line, sizeof(line));
        if (n <= 0) break;

        // Check for END marker
        if (strcmp(line, END_MARKER) == 0) break;

        int len = strlen(line);
        if (total + len >= size - 1) break;

        strcat(buf + total, line);
        total += len;
    }

    return total;
}
