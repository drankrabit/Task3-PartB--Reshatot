#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include "RUDP_API.h"
#include <sys/time.h>
#include <sys/select.h>
#include <errno.h>

#define HEADER_SIZE 5

int rudp_socket(int port) {
    int sockfd;
    struct sockaddr_in servaddr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        return -1;
    }

    // Allow address reuse
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(sockfd);
        return -1;
    }

    // Initialize server address structure
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("Bind failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int rudp_handshake(int sockfd, struct sockaddr *addr, socklen_t addrlen) {
    char handshake_msg[5]; // Assuming handshake message size is 5 bytes

    // Receive handshake message
    ssize_t bytes_received = recvfrom(sockfd, handshake_msg, sizeof(handshake_msg), 0, addr, &addrlen);
    if (bytes_received == -1) {
        perror("Error receiving handshake message");
        return -1;
    }

    printf("Handshake message received\n");

    // Send handshake acknowledgment
    char ack_msg[5] = "ACK"; // Example acknowledgment message
    if (sendto(sockfd, ack_msg, sizeof(ack_msg), 0, addr, addrlen) == -1) {
        perror("Error sending handshake acknowledgment");
        return -1;
    }

    printf("Handshake acknowledgment sent\n");

    return 0;
}

int rudp_send(int sockfd, struct sockaddr *addr, socklen_t addrlen, const void *data, size_t length) {
    // Check if the data is the "exit" message
    if (strcmp(data, "exit") == 0) {
        // Send "exit" message
        ssize_t bytes_sent = sendto(sockfd, data, strlen(data) + 1, 0, addr, addrlen);
        if (bytes_sent == -1) {
            perror("Error sending exit message");
            return -1;
        }
        return 0;
    }
    // Otherwise, send data as usual
    else {
        ssize_t bytes_sent = sendto(sockfd, data, length, 0, addr, addrlen);
        if (bytes_sent == -1) {
            perror("Error sending data");
            return -1;
        }
        return 0;
    }
}

int rudp_recv(int sockfd, struct sockaddr *addr, socklen_t *addrlen, void *buffer, size_t length) {
    // Implementation of rudp_recv
    // For simplicity, I'll provide a basic implementation without header parsing.
    ssize_t bytes_received = recvfrom(sockfd, buffer, length, 0, addr, addrlen);
    if (bytes_received == -1) {
        perror("Error receiving data");
        return -1;
    }
    return bytes_received;
}

int rudp_close(int sockfd) {
    // Close socket
    if (close(sockfd) == -1) {
        perror("Error closing socket");
        return -1;
    }
    return 0;
}
