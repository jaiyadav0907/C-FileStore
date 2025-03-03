#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "client.h"
#include "utils.h"

#define BUFFER_SIZE 1024

void printUsage() {
    printf("Usage:\n");
    printf("  PUT <bucket> <object> <data>   - Upload an object to a bucket\n");
    printf("  GET <bucket> <object>          - Download an object from a bucket\n");
    printf("  DELETE <bucket> <object>       - Delete an object from a bucket\n");
    printf("  LIST <bucket> <object>         - List all the objects in the bucket\n");
    printf("  EXIT                           - Exit the application\n");
}

int main() {
    char serverIP[INET_ADDRSTRLEN];
    int port;

    printf("Enter server IP address: ");
    scanf("%s", serverIP);
    printf("Enter server port: ");
    scanf("%d", &port);

    if (connectToServer(serverIP, port) < 0) {
        fprintf(stderr, "Failed to connect to server at %s:%d\n", serverIP, port);
        return EXIT_FAILURE;
    }

    char command[BUFFER_SIZE];
    while (1) {
        printf("Enter command: ");
        fgets(command, BUFFER_SIZE, stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline character

        char cmd[BUFFER_SIZE], bucket[BUFFER_SIZE], object[BUFFER_SIZE], data[BUFFER_SIZE];
        parseInput(command, cmd, bucket, object, data);

        if (strcmp(cmd, "EXIT") == 0) {
            break;
        } else if (strcmp(cmd, "PUT") == 0) {
            char request[BUFFER_SIZE];
            formatRequest(cmd, bucket, object, data, request);
            sendRequest(request);
        } else if (strcmp(cmd, "GET") == 0) {
            char request[BUFFER_SIZE];
            formatRequest(cmd, bucket, object, NULL, request);
            sendRequest(request);
        } else if (strcmp(cmd, "DELETE") == 0) {
            char request[BUFFER_SIZE];
            formatRequest(cmd, bucket, object, NULL, request);
            sendRequest(request);
        } else if (strcmp(cmd, "LIST") == 0) {
            char request[BUFFER_SIZE];
            formatRequest(cmd, bucket, NULL, NULL, request);
            sendRequest(request);
        } else {
            printUsage();
        }

        char* response = receiveResponse();
        if (response) {
            displayResponse(response);
            free(response);
        }
    }

    closeConnection();
    return EXIT_SUCCESS;
}