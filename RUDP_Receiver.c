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
#define TIMEOUT_SEC 10

void print_statistics(double *time_taken, double *bandwidth, int num_runs) {
    double avg_time = 0.0, avg_bandwidth = 0.0;
    printf("----------------------------------\n");
    printf("- * Statistics * -\n");
    // Starts to loop to print all of the statistics for each run
    for (int i = 0; i < num_runs; i++) {
        printf("- Run #%d Data: Time=%.6f seconds; Speed=%.2f Bytes/s\n", i + 1, time_taken[i], bandwidth[i]);
        // Sums all the times and bandwidths for average calculation
        avg_time += time_taken[i];
        avg_bandwidth += bandwidth[i];
    }
    // Calculates the average
    avg_time /= num_runs;
    avg_bandwidth /= num_runs;
    printf("-\n");
    printf("- Average time: %.6f seconds\n", avg_time);
    printf("- Average bandwidth: %.2f Bytes/s\n", avg_bandwidth);
    printf("----------------------------------\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3 || strcmp(argv[1], "-p") != 0) {
        fprintf(stderr, "Usage: %s -p PORT\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[2]);

    int sockfd;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    double time_taken[100]; // Array to store time taken for each file transfer
    double bandwidth[100];  // Array to store bandwidth for each file transfer
    int run_count = 0;      // Counter for number of runs

    printf("Starting Receiver\n");
    printf("Waiting for RUDP connection...\n");

    // Create RUDP socket
    sockfd = rudp_socket(port);
    if (sockfd == -1) {
        fprintf(stderr, "Failed to create RUDP socket\n");
        exit(EXIT_FAILURE);
    }

    // Receive handshake message
    char handshake_msg[5];
    ssize_t bytes_received = recvfrom(sockfd, handshake_msg, sizeof(handshake_msg), 0, (struct sockaddr *)&cliaddr, &clilen);
    if (bytes_received == -1) {
        perror("Error receiving handshake message");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Handshake message received\n");

    // Send handshake acknowledgment
    char ack_msg[5] = "ACK"; // Acknowledgment message
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

                // Calculate time taken for file transfer
                struct timeval start, end;
                gettimeofday(&start, NULL);
                
                printf("File transfer completed.\n");

                // Send acknowledgment message back to the sender
                char ack_msg[4] = "ACK"; // Acknowledgment message
                if (rudp_send(sockfd, (struct sockaddr *)&cliaddr, clilen, ack_msg, strlen(ack_msg)) == -1) {
                    fprintf(stderr, "Error sending acknowledgment\n");
                    rudp_close(sockfd);
                    exit(EXIT_FAILURE);
                }

                gettimeofday(&end, NULL);

                // Calculate time taken for file transfer and bandwidth
                double time_taken_seconds = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000;
                double file_size = bytes_received;
                double speed = file_size / time_taken_seconds;

                time_taken[run_count] = time_taken_seconds;
                bandwidth[run_count] = speed;
                run_count++;

                printf("Acknowledgment sent\n");
                printf("Waiting for Sender response...\n");
                if (strcmp(received_data, "exit") == 0) {
                    printf("Sender sent exit message\n");
                    break;
                }
                printf("Sender sent continue message...\n");
            }
        }
    }

    // Close connection
    if (rudp_close(sockfd) == -1) {
        fprintf(stderr, "Failed to close RUDP connection\n");
        exit(EXIT_FAILURE);
    }

    // Print statistics
    print_statistics(time_taken, bandwidth, run_count);

    return 0;
}