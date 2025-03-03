#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "client.h"
#include "utils.h"

#define BUFFER_SIZE 1024

void printUsage() {
    printf("Usage:\n");
    printf("  PUT <bucket> <object> <file>   - Upload a local file as an object to a bucket\n");
    printf("  GET <bucket> <object> <file>   - Download an object from a bucket to a local file\n");
    printf("  DELETE <bucket> <object>       - Delete an object from a bucket\n");
    printf("  LIST <bucket> <object>         - List all the objects in the bucket\n");
    printf("  EXIT                           - Exit the application\n");
}

int main() {
    char serverIP[INET_ADDRSTRLEN];
    int port;

    printf("Enter server IP and port (format: <ipaddress>:<port>): ");
    char input[BUFFER_SIZE];
    if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
        fprintf(stderr, "Failed to read input.\n");
        return EXIT_FAILURE;
    }
    if (sscanf(input, "%15[^:]:%d", serverIP, &port) != 2) {
        fprintf(stderr, "Invalid format. Please use the format: <ipaddress>:<port>\n");
        return EXIT_FAILURE;
    }
    
    if (connectToServer(serverIP, port) < 0) {
        fprintf(stderr, "Failed to connect to server at %s:%d\n", serverIP, port);
        return EXIT_FAILURE;
    }

    char command[BUFFER_SIZE];
    while (1) {
        printf("file-storage> ");
        fgets(command, BUFFER_SIZE, stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline character

        char cmd[BUFFER_SIZE], bucket[BUFFER_SIZE], object[BUFFER_SIZE], data[BUFFER_SIZE], request[BUFFER_SIZE];
        parseInput(command, cmd, bucket, object, data);

        if (strcmp(cmd, "EXIT") == 0) {
            break;
        } else if (strcmp(cmd, "PUT") == 0) {
            formatRequest(cmd, bucket, object, data, request);
            sendRequest(request);
            char* response = receiveResponse();
            if (response) {
                displayResponse(response);
                free(response);
            }
        } else if (strcmp(cmd, "GET") == 0) {
            formatRequest(cmd, bucket, object, NULL, request);
            sendRequest(request);
            char *response = receiveResponse();
            if (response) {
                FILE *file = fopen(data, "wb");
                if (file) {
                    fwrite(response, 1, strlen(response), file);
                    fclose(file);
                    printf("File downloaded successfully.\n");
                } else {
                    perror("Failed to open file for writing.\n");
                }
                free(response);
            }
            
        } else if (strcmp(cmd, "DELETE") == 0) {
            formatRequest(cmd, bucket, object, NULL, request);
            sendRequest(request);
            char* response = receiveResponse();
            if (response) {
                displayResponse(response);
                free(response);
            }
        } else if (strcmp(cmd, "LIST") == 0) {
            formatRequest(cmd, bucket, NULL, NULL, request);
            sendRequest(request);
            char* response = receiveResponse();
            if (response) {
                displayResponse(response);
                free(response);
            }
        } else {
            printUsage();
        }
    }

    closeConnection();
    return EXIT_SUCCESS;
}