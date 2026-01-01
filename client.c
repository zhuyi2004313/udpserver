#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

static void die(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <port> <message>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *server_ip = argv[1];

    char *endptr = NULL;
    long port = strtol(argv[2], &endptr, 10);
    if (argv[2][0] == '\0' || *endptr != '\0' || port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port: %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    const char *message = argv[3];

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        die("socket");
    }

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        die("setsockopt");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server IP: %s\n", server_ip);
        close(sockfd);
        return EXIT_FAILURE;
    }

    ssize_t sent = sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&server_addr,
                          sizeof(server_addr));
    if (sent < 0) {
        die("sendto");
    }

    printf("Sent %zd bytes to %s:%ld\n", sent, server_ip, port);

    char buffer[BUFFER_SIZE];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    ssize_t received = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                (struct sockaddr *)&from_addr, &from_len);

    if (received < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            fprintf(stderr, "Timed out waiting for response.\n");
        } else {
            die("recvfrom");
        }
        close(sockfd);
        return EXIT_FAILURE;
    }

    buffer[received] = '\0';

    char from_ip[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &from_addr.sin_addr, from_ip, sizeof(from_ip)) == NULL) {
        strncpy(from_ip, "unknown", sizeof(from_ip));
        from_ip[sizeof(from_ip) - 1] = '\0';
    }

    printf("Received response from %s:%d -> %s\n", from_ip, ntohs(from_addr.sin_port), buffer);

    close(sockfd);
    return EXIT_SUCCESS;
}
