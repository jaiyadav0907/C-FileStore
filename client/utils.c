#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parseInput(char* input, char* command, char* bucket, char* object, char* data) {
    char* token = strtok(input, " ");
    if (token != NULL) {
        strcpy(command, token);
        token = strtok(NULL, " ");
        if (token != NULL) {
            strcpy(bucket, token);
            token = strtok(NULL, " ");
            if (token != NULL) {
                strcpy(object, token);
                token = strtok(NULL, "\n");
                if (token != NULL) {
                    strcpy(data, token);
                }
            }
        }
    }
}

void formatRequest(const char* command, const char* bucket, const char* object, const char* data, char* request) {
    if (strcmp(command, "PUT") == 0) {
        snprintf(request, MAX_BUFFER_SIZE, "PUT /%s/%s HTTP/1.1\r\nContent-Length: %zu\r\n\r\n%s", bucket, object, strlen(data), data);
    } else if (strcmp(command, "GET") == 0) {
        snprintf(request, MAX_BUFFER_SIZE, "GET /%s/%s HTTP/1.1\r\n\r\n", bucket, object);
    } else if (strcmp(command, "DELETE") == 0) {
        snprintf(request, MAX_BUFFER_SIZE, "DELETE /%s/%s HTTP/1.1\r\n\r\n", bucket, object);
    } else if (strcmp(command, "LIST") == 0) {
        snprintf(request, MAX_BUFFER_SIZE, "GET /%s HTTP/1.1\r\n\r\n", bucket);
    } else {
        snprintf(request, MAX_BUFFER_SIZE, "INVALID COMMAND");
    }
}

void displayResponse(const char* response) {
    printf("Server Response:\n%s\n", response);
}