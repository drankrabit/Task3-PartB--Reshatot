#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "RUDP_API.h"

#define MAXLINE 1024

int main(int argc, char *argv[]) {
    if (argc != 3 || strcmp(argv[1], "-p") != 0) {
        fprintf(stderr, "Usage: %s -p PORT\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[2]);

    int sockfd;

    printf("Starting Receiver\n");
    printf("Waiting for RUDP connection...\n");
    
    // Create RUDP socket
    sockfd = rudp_socket(port);
    if (sockfd == -1) {
        fprintf(stderr, "Failed to create RUDP socket\n");
        exit(EXIT_FAILURE);
    }

    printf("Shutting down Receiver\n");

    // Close connection
    if (rudp_close(sockfd) == -1) {
        fprintf(stderr, "Failed to close RUDP connection\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
