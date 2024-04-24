#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include "RUDP_API.h"

#define MAXLINE 1024

char *util_generate_random_data(unsigned int size) {
    char *buffer = NULL;
    // Argument check.
    if (size == 0)
        return NULL;
    buffer = (char *)calloc(size, sizeof(char));
    // Error checking.
    if (buffer == NULL)
        return NULL;
    // Randomize the seed of the random number generator.
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++)
        *(buffer + i) = ((unsigned int)rand() % 256);
    return buffer;
}

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

    while (1) {
        // Generate random data
        unsigned int data_size = 1024; // Adjust as needed
        char *random_data = util_generate_random_data(data_size);
        if (random_data == NULL) {
            fprintf(stderr, "Failed to generate random data\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // Send file using RUDP
        if (rudp_send(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr), random_data, data_size) == -1) {
            fprintf(stderr, "Error sending file\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        printf("File sent successfully\n");
        free(random_data);

        // Ask user if they want to send another file
        char user_input[MAXLINE];
        printf("Do you want to send another file? (Y/N): ");
        fgets(user_input, MAXLINE, stdin);
        strtok(user_input, "\n"); // Remove newline character

        // Check if sender should exit
        if (strcmp(user_input, "N") == 0 || strcmp(user_input, "n") == 0) {
            // Send user choice to receiver
            if (rudp_send(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr), "exit", strlen("exit") + 1) == -1) {
                perror("Error sending exit message");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            printf("Exit message sent. Sender end.\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}
