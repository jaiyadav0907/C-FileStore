#ifndef UTILS_H
#define UTILS_H

void parseInput(char* input, char* command, char* bucket, char* object, char* data);
void formatRequest(const char* command, const char* bucket, const char* object, const char* data, const char* request);
void displayResponse(const char* response);

#endif