#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>
#include <errno.h>
#include "RUDP_API.h"

#define MAXLINE 1024
#define TIMEOUT_SEC 5

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
    ssize_t bytes_received = recvfrom(sockfd, handshake_msg, sizeof(handshake_msg), 0, (struct sockaddr *)&cliaddr, &clilen);
    if (bytes_received == -1) {
        perror("Error receiving handshake message");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Handshake message received\n");

    // Send handshake acknowledgment
    char ack_msg[5] = "ACK"; // Example acknowledgment message
    if (sendto(sockfd, ack_msg, sizeof(ack_msg), 0, (struct sockaddr *)&cliaddr, clilen) == -1) {
        perror("Error sending handshake acknowledgment");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Handshake acknowledgment sent\n");

    printf("Handshake successful\n");

    printf("Waiting for data...\n");

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        // Wait for data or timeout
        int activity = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0) {
            perror("select error");
            exit(EXIT_FAILURE);
        } else if (activity == 0) {
            printf("Timeout occurred. No data received from sender.\n");
            break;
        } else {
            if (FD_ISSET(sockfd, &readfds)) {
                // Receive file using RUDP
                char received_data[MAXLINE];
                int bytes_received = rudp_recv(sockfd, (struct sockaddr *)&cliaddr, &clilen, received_data, MAXLINE);
                if (bytes_received == -1) {
                    fprintf(stderr, "Error receiving file\n");
                    rudp_close(sockfd);
                    exit(EXIT_FAILURE);
                }

                printf("Received file from sender\n");

                // Send acknowledgment message back to the sender
                char ack_msg[4] = "ACK"; // Acknowledgment message
                if (rudp_send(sockfd, (struct sockaddr *)&cliaddr, clilen, ack_msg, strlen(ack_msg)) == -1) {
                    fprintf(stderr, "Error sending acknowledgment\n");
                    rudp_close(sockfd);
                    exit(EXIT_FAILURE);
                }

                printf("Acknowledgment sent\n");

                if (strcmp(received_data, "exit") == 0) {
                    printf("Sender sent exit message\n");
                    break;
                }
            }
        }
    }

    // Close connection
    if (rudp_close(sockfd) == -1) {
        fprintf(stderr, "Failed to close RUDP connection\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
