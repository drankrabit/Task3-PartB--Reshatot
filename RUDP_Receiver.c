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
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    printf("Starting Receiver\n");
    printf("Waiting for RUDP connection...\n");

    // Create RUDP socket
    sockfd = rudp_socket(port);
    if (sockfd == -1) {
        fprintf(stderr, "Failed to create RUDP socket\n");
        exit(EXIT_FAILURE);
    }

    // Receive handshake message
    char handshake_msg[5]; // Assuming handshake message size is 5 bytes
    socklen_t addrlen = sizeof(cliaddr);
    int bytes_received = recvfrom(sockfd, handshake_msg, sizeof(handshake_msg), 0, (struct sockaddr *)&cliaddr, &addrlen);
    if (bytes_received == -1) {
        perror("Error receiving handshake message");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Handshake message received\n");

    // Send handshake acknowledgment
    char ack_msg[5] = "ACK"; // Example acknowledgment message
    if (sendto(sockfd, ack_msg, sizeof(ack_msg), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) == -1) {
        perror("Error sending handshake acknowledgment");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Handshake acknowledgment sent\n");

    printf("Waiting for data...\n");

    // Receive file using RUDP
    char received_data[MAXLINE];
    bytes_received = rudp_recv(sockfd, (struct sockaddr *)&cliaddr, &clilen, received_data, MAXLINE);
    if (bytes_received == -1) {
        fprintf(stderr, "Error receiving file\n");
        rudp_close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("File received successfully\n");

    // Close connection
    if (rudp_close(sockfd) == -1) {
        fprintf(stderr, "Failed to close RUDP connection\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
