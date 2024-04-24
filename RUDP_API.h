#ifndef RUDP_API_H
#define RUDP_API_H

#include <netinet/in.h>
#include <sys/types.h>

// Define packet structure
typedef struct {
    int seq_num;
    char data[1024]; // Adjust the size if needed
} Packet;

// Define ACK packet structure
typedef struct {
    int seq_num;
} ACK;

// RUDP socket creation and performing a handshake
int rudp_socket(int port);

// Handshake with peer
int rudp_handshake(int sockfd, struct sockaddr *addr, socklen_t addrlen);

// Send data to the peer
int rudp_send(int sockfd, struct sockaddr *addr, socklen_t addrlen, const void *data, size_t length);

// Receive data from a peer
int rudp_recv(int sockfd, struct sockaddr *addr, socklen_t *addrlen, void *buffer, size_t length);

// Close a connection between peers
int rudp_close(int sockfd);

#endif /* RUDP_API_H */
