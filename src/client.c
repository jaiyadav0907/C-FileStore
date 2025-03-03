#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

static int socket_fd;

int connectToServer(const char* ip, int port) {
    struct sockaddr_in server_address;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_address.sin_addr);

    if (connect(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Connection to server failed");
        close(socket_fd);
        return -1;
    }

    return 0;
}

void sendRequest(const char* request) {
    send(socket_fd, request, strlen(request), 0);
}

void receiveResponse() {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Response from server:\n%s\n", buffer);
    } else {
        perror("Failed to receive response");
    }
}

void closeConnection() {
    close(socket_fd);
}