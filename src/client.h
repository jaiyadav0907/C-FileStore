#ifndef CLIENT_H
#define CLIENT_H

#include <stddef.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_IP_LENGTH 16
#define MAX_PORT_LENGTH 6

// Function prototypes
int connectToServer(const char* ipAddress, const char* port);
void sendRequest(int socket, const char* request);
char* receiveResponse(int socket, size_t* responseSize);
void closeConnection(int socket);

#endif // CLIENT_H