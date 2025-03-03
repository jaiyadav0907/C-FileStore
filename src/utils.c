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
        snprintf(request, BUFFER_SIZE, "PUT /%s/%s\r\n\r\n%s", bucket, object, data);
    } else if (strcmp(command, "GET") == 0) {
        snprintf(request, BUFFER_SIZE, "GET /%s/%s\r\n", bucket, object);
    } else if (strcmp(command, "DELETE") == 0) {
        snprintf(request, BUFFER_SIZE, "DELETE /%s/%s\r\n", bucket, object);
    } else {
        snprintf(request, BUFFER_SIZE, "INVALID COMMAND");
    }
}

void displayResponse(const char* response) {
    printf("Server Response:\n%s\n", response);
}