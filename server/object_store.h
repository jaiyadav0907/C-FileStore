#ifndef OBJECT_STORE_H
#define OBJECT_STORE_H


#define MAX_BUCKET_NAME 100
#define MAX_OBJECT_NAME 100
#define BUFFER_SIZE 4096 
#define STORAGE_DIR "storage"
#define THREAD_POOL_SIZE 4

#include <stdlib.h>

void ensureStorageDirExists();

void createBucket(const char* bucket);

void uploadObject(const char* bucket, const char* object, const char* data, size_t size);

char* downloadObject(const char* bucket, const char* object, size_t* size);

void deleteObject(const char* bucket, const char* object);

char* listObjects(const char* bucket);

#endif