#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "RUDP_API.h"

#define MAXLINE 1024

int main(int argc, char *argv[]) {
    if (argc != 4 || strcmp(argv[1], "-p") != 0) {
        fprintf(stderr, "Usage: %s -p PORT <IP>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[2]);
    char *ip = argv[3];

    int sockfd;
    struct sockaddr_in servaddr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(ip);

    // Send handshake message
    char handshake_msg[5] = "HELLO"; // Example handshake message
    if (sendto(sockfd, handshake_msg, strlen(handshake_msg), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("Error sending handshake message");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Handshake message sent\n");

    // Receive handshake acknowledgment
    char ack_msg[5]; // Assuming acknowledgment message size is 5 bytes
    socklen_t addrlen = sizeof(servaddr);
    int bytes_received = recvfrom(sockfd, ack_msg, sizeof(ack_msg), 0, (struct sockaddr *)&servaddr, &addrlen);
    if (bytes_received == -1) {
        perror("Error receiving handshake acknowledgment");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Handshake successful\n");

    // Close connection
    if (rudp_close(sockfd) == -1) {
        fprintf(stderr, "Failed to close RUDP connection\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}