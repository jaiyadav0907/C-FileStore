#include "object_store.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h> 
#include <sys/stat.h>
#include <dirent.h>

void ensureStorageDirExists() {
    struct stat st = {0};
    if (stat(STORAGE_DIR, &st) == -1) {
        mkdir(STORAGE_DIR, 0700); 
    }
}

void getObjectPath(const char* bucket, const char* object, char* path) {
    snprintf(path, BUFFER_SIZE, "%s/%s/%s", STORAGE_DIR, bucket, object);
}

void createBucket(const char* bucket) {
    char path[BUFFER_SIZE];
    snprintf(path, BUFFER_SIZE, "%s/%s", STORAGE_DIR, bucket);
    mkdir(path, 0700);
}

void uploadObject(const char* bucket, const char* object, const char* data, size_t size) {
    char path[BUFFER_SIZE];
    getObjectPath(bucket, object, path);
    
    FILE* file = fopen(path, "wb");
    if (file == NULL) {
        perror("Failed to open file for writing.");
        return;
    }
    fwrite(data, 1, size, file);
    fclose(file);
}

char* downloadObject(const char* bucket, const char* object, size_t* size) {
    char path[BUFFER_SIZE];
    getObjectPath(bucket, object, path);

    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        perror("Failed to open file for reading.");
        return NULL;
    }
    // meant for getting the file size
    struct stat st;
    if (stat(path, &st) == 0) {
        *size = st.st_size;
    } else {
        perror("Failed to get file size.");
        fclose(file);
        return NULL;
    }

    char* data = malloc(*size);
    if (data == NULL) {
        perror("Failed to allocate memory for object.");
        fclose(file);
        return NULL;
    }

    fread(data, 1, *size, file);
    fclose(file);
    return data;
}

void deleteObject(const char* bucket, const char* object) {
    char path[BUFFER_SIZE];
    getObjectPath(bucket, object, path);
    if (remove(path) != 0) {
        perror("Failed to delete object.");
    }
}

char* listObjects(const char* bucket) { 
    char path[BUFFER_SIZE];
    snprintf(path, BUFFER_SIZE, "%s/%s", STORAGE_DIR, bucket);

    DIR* dir = opendir(path);
    if (dir == NULL) {
        perror("Failed to open directory.");
        return NULL;
    }

    char* result = (char*)malloc(BUFFER_SIZE);
    if (result == NULL) {
        perror("Failed to allocate memory for listing objects.");
        closedir(dir);
        return NULL;
    }

    result[0] = '\0';
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            strcat(result, entry->d_name);
            strcat(result, "\n");
        }
    }
    closedir(dir);
    return result;
}