#ifndef LOGGER_H
#define LOGGER_H

#define LOG_LEVEL_INFO "INFO"
#define LOG_LEVEL_WARN "WARN"
#define LOG_LEVEL_ERROR "ERROR"

void initLogger(const char* logFilePath);
void closeLogger();
void logMessage(const char* level, const char* format, ...);

#endif

