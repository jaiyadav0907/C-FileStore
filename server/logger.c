#include "logger.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static FILE* logFile = NULL;

void initLogger(const char* logFilePath) {
    logFile = fopen(logFilePath, "a");
    if (!logFile) {
        perror("Failed to open log file.");
        exit(EXIT_FAILURE);
    }
}

void closeLogger() {
    if (logFile) {
        fclose(logFile);
        logFile = NULL;
    }
}

void logMessage(const char* level, const char* format, ...) {
    if (!level || !format) {
        fprintf(stderr, "Error: Invalid arguments to logMessage (level: %p, format: %p)\n", (void*)level, (void*)format);
        return;
    }

    time_t now;
    time(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    char message[1024];
    snprintf(message, sizeof(message), "[%s] [%s] ", timestamp, level);

    va_list args;
    va_start(args, format);
    vsnprintf(message + strlen(message), sizeof(message) - strlen(message), format, args);
    va_end(args);

    printf("%s\n", message);
    if (logFile) {
        fprintf(logFile, "%s\n", message);
        fflush(logFile);
    }
}