#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

static volatile sig_atomic_t keep_running = 1;

static void handle_interrupt(int signum) {
    (void)signum;
    keep_running = 0;
}

static void die(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *endptr = NULL;
    long port = strtol(argv[1], &endptr, 10);
    if (argv[1][0] == '\0' || *endptr != '\0' || port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        die("socket");
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        die("setsockopt");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons((uint16_t)port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        die("bind");
    }

    printf("UDP server listening on port %ld...\n", port);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_interrupt;
    sigaction(SIGINT, &sa, NULL);

    char buffer[BUFFER_SIZE];
    while (keep_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        ssize_t received = recvfrom(
            sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_len);

        if (received < 0) {
            if (errno == EINTR) {
                continue;  // Interrupted by signal, check loop condition.
            }
            die("recvfrom");
        }

        buffer[received] = '\0';

        char client_ip[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip)) == NULL) {
            strncpy(client_ip, "unknown", sizeof(client_ip));
            client_ip[sizeof(client_ip) - 1] = '\0';
        }

        printf("Received %zd bytes from %s:%d -> %s\n", received, client_ip,
               ntohs(client_addr.sin_port), buffer);

        const char *response = "Message received";
        if (sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr,
                   client_len) < 0) {
            perror("sendto");
        }
    }

    printf("Shutting down server.\n");
    close(sockfd);
    return EXIT_SUCCESS;
}
