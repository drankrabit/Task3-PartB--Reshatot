#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "RUDP_API.h"

int rudp_socket(int port) {
    int sockfd;
    struct sockaddr_in servaddr, sender_addr;
    char handshake_msg[5]; // Assuming handshake message size is 5 bytes

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
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

    // Wait for handshake from sender
    socklen_t sender_addrlen = sizeof(sender_addr);
    int bytes_received = recvfrom(sockfd, handshake_msg, sizeof(handshake_msg), 0, (struct sockaddr *)&sender_addr, &sender_addrlen);
    if (bytes_received == -1) {
        perror("Error receiving handshake request");
        close(sockfd);
        return -1;
    }

    printf("Connection request recieved, sending ACK\n");

    // Respond with a handshake message
    strcpy(handshake_msg, "ACK"); // Example response
    if (sendto(sockfd, handshake_msg, strlen(handshake_msg), 0, (struct sockaddr *)&sender_addr, sender_addrlen) == -1) {
        perror("Error sending handshake response");
        close(sockfd);
        return -1;
    }

    printf("Connection established successfully\n");

    return sockfd;
}

int rudp_close(int sockfd) {
    // Close socket
    if (close(sockfd) == -1) {
        perror("Error closing socket");
        return -1;
    }
    printf("Connection closed\n");
    return 0;
}
