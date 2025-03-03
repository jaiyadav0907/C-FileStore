#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"

#define BUFFER_SIZE 1024

void printUsage() {
    printf("Usage:\n");
    printf("  put <bucket> <object> <data>   - Upload an object to a bucket\n");
    printf("  get <bucket> <object>          - Download an object from a bucket\n");
    printf("  delete <bucket> <object>       - Delete an object from a bucket\n");
    printf("  exit                           - Exit the application\n");
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

        if (strcmp(command, "exit") == 0) {
            break;
        } else if (strncmp(command, "put", 3) == 0) {
            handlePutCommand(command);
        } else if (strncmp(command, "get", 3) == 0) {
            handleGetCommand(command);
        } else if (strncmp(command, "delete", 6) == 0) {
            handleDeleteCommand(command);
        } else {
            printUsage();
        }
    }

    closeConnection();
    return EXIT_SUCCESS;
}