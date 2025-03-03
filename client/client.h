#ifndef CLIENT_H
#define CLIENT_H

#include <stddef.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_IP_LENGTH 16
#define MAX_PORT_LENGTH 6

int connectToServer(const char* ipAddress, int port);
void sendRequest(const char* request);
char* receiveResponse();
void closeConnection();

#endif // CLIENT_H