

#include "http_server.h"
#include "object_store.h"
#include "logger.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

typedef struct {
    int clientSocket;
    char clientIP[INET_ADDRSTRLEN];
} Task;

typedef struct {
    Task* tasks;
    int capacity;
    int size;
    int front;
    int rear;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    bool shutdown;
    pthread_t threads[THREAD_POOL_SIZE];
} threadPool;

threadPool globalThreadPool;


void handleHttpRequest(int clientSocket, const char* clientIP) {
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesRead <= 0) {
        logMessage(LOG_LEVEL_ERROR, "[Client: %s] Failed to read from client socket.", clientIP);
        close(clientSocket);
        return;
    }
    buffer[bytesRead] = '\0';
    char method[10], path[BUFFER_SIZE];
    sscanf(buffer, "%s %s", method, path);
    logMessage(LOG_LEVEL_INFO, "[Client: %s] Incoming request. Method: %s. Path: %s", clientIP, method, path);

    char response[BUFFER_SIZE];
    if (strcmp(method, "PUT") == 0) {
        char bucket[MAX_BUCKET_NAME], object[MAX_OBJECT_NAME];
        if (sscanf(path, "/%[^/]/%s", bucket, object) != 2) {
            snprintf(response, BUFFER_SIZE, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
            logMessage(LOG_LEVEL_ERROR, "[Client: %s] Invalid request body.", clientIP);
        } else {
            const char* body = strstr(buffer, "\r\n\r\n");
            if (body) {
                body += 4; //skipping the \r\n\r\n to get to the real body
                createBucket(bucket);
                uploadObject(bucket, object, body, bytesRead - (body - buffer));
                snprintf(response, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");
                logMessage(LOG_LEVEL_INFO, "[Client: %s] Object uploaded successfully. [Bucket: %s]. [Object: %s]", clientIP, bucket, object);
            } else {
                snprintf(response, BUFFER_SIZE, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
                logMessage(LOG_LEVEL_ERROR, "[Client: %s] Invalid request body.", clientIP);
            }
        }

    } else if (strcmp(method, "GET") == 0) {
        char bucket[MAX_BUCKET_NAME], object[MAX_BUCKET_NAME];
        if (sscanf(path, "/%[^/]/%s", bucket, object) == 2) {
            size_t size;
            char* data = downloadObject(bucket, object, &size);
            if (data) {
                snprintf(response, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nContent-Length: %zu\r\n\r\n", size);
                send(clientSocket, response, strlen(response), 0);
                send(clientSocket, data, size, 0);
                free(data);
                logMessage(LOG_LEVEL_INFO, "[Client: %s] Object downloaded successfully. [Bucket: %s]. [Object: %s]", clientIP, bucket, object);
                close(clientSocket);
                return;
            } else {
                snprintf(response, BUFFER_SIZE, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
                logMessage(LOG_LEVEL_ERROR, "[Client: %s] Object not found. [Bucket: %s]. [Object: %s]", clientIP, bucket, object);
            }
        } else if (sscanf(path, "/%[^/]/%s", bucket) == 1) { 
            char* objects = listObjects(bucket);
            if (objects) {
                snprintf(response, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nContent-Length: %zu\r\n\r\n%s", strlen(objects), objects);
                send(clientSocket, response, strlen(response), 0);
                logMessage(LOG_LEVEL_INFO, "[Client: %s] List of objects retreived. [Bucket: %s]", clientIP, bucket);
                free(objects);
            } else {
                logMessage(LOG_LEVEL_ERROR, "[Client: %s] Bucket not found. [Bucket: %s]", clientIP, bucket);
                snprintf(response, BUFFER_SIZE, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
            }
        }
    } else if (strcmp(method, "DELETE") == 0) {
        char bucket[MAX_BUCKET_NAME], object[MAX_OBJECT_NAME];
        if (sscanf(path, bucket, object) == 2) {
            deleteObject(bucket, object);
            snprintf(response, BUFFER_SIZE, "HTTP/1.1 204 No Content\r\nContent-Length: 0\r\n");
            logMessage(LOG_LEVEL_INFO, "[Client: %s] Object deleted succesfully. [Bucket: %s]. [Object: %s]", clientIP, bucket, object);
        } else {
            snprintf(response, BUFFER_SIZE, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n");
            logMessage(LOG_LEVEL_ERROR, "[Client: %s] Invalid delete request.", clientIP);
        }
    } else {
        snprintf(response, BUFFER_SIZE, "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n");
        logMessage(LOG_LEVEL_ERROR, "[Client: %s] Unsupported HTTP Method request. Method: %s", clientIP, method);
    }
    send(clientSocket, response, strlen(response), 0);
    close(clientSocket);
    logMessage(LOG_LEVEL_INFO, "[Client: %s] Request processed.", clientIP);
}


void* WorkerThread(void* arg) {
    int thread_id = *(int*)arg;
    while(1) {
        pthread_mutex_lock(&globalThreadPool.lock);
        while(globalThreadPool.size == 0 && !globalThreadPool.shutdown) {
            pthread_cond_wait(&globalThreadPool.cond, &globalThreadPool.lock);
        }
        if (globalThreadPool.shutdown) {
            pthread_mutex_unlock(&globalThreadPool.lock);
            logMessage(LOG_LEVEL_INFO, "[Thread-%d] shutting down.", thread_id);
            break;
        }

        Task task = globalThreadPool.tasks[globalThreadPool.front];
        globalThreadPool.front = (globalThreadPool.front + 1) % globalThreadPool.capacity;
        globalThreadPool.size--;
        pthread_mutex_unlock(&globalThreadPool.lock);

        logMessage(LOG_LEVEL_INFO, "[Thread-%d] Processing task. [Client: %s]", thread_id, task.clientIP);
        handleHttpRequest(task.clientSocket, task.clientIP);
    }  return NULL;
}   

void threadPool_init(int capacity) {
    logMessage(LOG_LEVEL_INFO, "Initializing thread pool.....");
    globalThreadPool.tasks = (Task*) malloc(capacity * sizeof(Task));
    if (!globalThreadPool.tasks) {
        logMessage(LOG_LEVEL_ERROR, "Failed to allocate memory for tasks queue.");
        exit(EXIT_FAILURE);
    }
    globalThreadPool.capacity = capacity;
    globalThreadPool.size = 0;
    globalThreadPool.front = 0;
    globalThreadPool.rear = 0;
    pthread_mutex_init(&globalThreadPool.lock, NULL);
    pthread_cond_init(&globalThreadPool.cond, NULL);
    globalThreadPool.shutdown = false;

    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        void* arg = &i;
        if (pthread_create(&globalThreadPool.threads[i], NULL, WorkerThread, arg) != 0) {
            logMessage(LOG_LEVEL_ERROR, "[Thread-%d] Failed to create thread.", i);
            exit(EXIT_FAILURE);
        }
    }
    logMessage(LOG_LEVEL_INFO, "Threadpool initalization completed.");
}

void threadPool_destroy() {
    logMessage(LOG_LEVEL_INFO, "Initiating thread pool destruction.....");
    pthread_mutex_lock(&globalThreadPool.lock);
    globalThreadPool.shutdown = true;
    pthread_cond_broadcast(&globalThreadPool.cond);
    pthread_mutex_unlock(&globalThreadPool.lock);

    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(globalThreadPool.threads[i], NULL);
    }

    free(globalThreadPool.tasks);
    pthread_mutex_destroy(&globalThreadPool.lock);
    pthread_cond_destroy(&globalThreadPool.cond);
    logMessage(LOG_LEVEL_INFO, "Thread pool destroyed.");
}

void threadPool_addtask(int clientSocket, const char* clientIP) {
    pthread_mutex_lock(&globalThreadPool.lock);
    if (globalThreadPool.size == globalThreadPool.capacity) {
        logMessage(LOG_LEVEL_WARN, "Task queue is full (%d/%d). Dropping client [Client: %s].", globalThreadPool.size, globalThreadPool.capacity, clientIP);
        close(clientSocket);
        pthread_mutex_unlock(&globalThreadPool.lock);
        return;
    }

    globalThreadPool.tasks[globalThreadPool.rear].clientSocket = clientSocket;
    strncpy(globalThreadPool.tasks[globalThreadPool.rear].clientIP, clientIP, INET_ADDRSTRLEN);
    globalThreadPool.tasks[globalThreadPool.rear].clientIP[INET_ADDRSTRLEN - 1] = '\0'; // Ensure null-termination
    globalThreadPool.rear = (globalThreadPool.rear + 1) % globalThreadPool.capacity;
    globalThreadPool.size++;

    pthread_cond_signal(&globalThreadPool.cond);
    pthread_mutex_unlock(&globalThreadPool.lock);

    logMessage(LOG_LEVEL_INFO, "Task added to threadpool [Client: %s]", clientIP);
}



void startHttpServer() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t addressLength = sizeof(serverAddress);
    char clientIP[INET_ADDRSTRLEN];

    ensureStorageDirExists();
    
    errno_t errno = 0;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        logMessage(LOG_LEVEL_ERROR, "Unable to create server socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; 
    serverAddress.sin_port = htons(PORT);
    
    if (bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        logMessage(LOG_LEVEL_ERROR, "Unable to bind server socket to localhost: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        logMessage(LOG_LEVEL_ERROR, "Listen failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
 
    logMessage(LOG_LEVEL_INFO, "Server listening on port %d....", PORT);
    
    threadPool_init(100);

    while(1) { 
        if ((clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddress, &addressLength)) < 0) {
            logMessage(LOG_LEVEL_ERROR, "Accept Failed: %s", strerror(errno));
            continue;
        }
        inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, INET_ADDRSTRLEN);
        logMessage(LOG_LEVEL_INFO, "New client connected [Client: %s].", clientIP);
        threadPool_addtask(clientSocket, clientIP);
    }

    threadPool_destroy();
    close(serverSocket);
    logMessage(LOG_LEVEL_INFO, "Server shutting down.");
    exit(EXIT_SUCCESS);

}