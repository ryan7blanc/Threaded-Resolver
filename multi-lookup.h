#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H

#include "util.h"
#include "array.h"

#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/sem.h>


#define MAX_INPUT_FILES 100
#define MAX_REQUESTER_THREADS 10
#define MAX_RESOLVER_THREADS 10
#define MAX_IP_LENGTH INET6_ADDRSTRLEN
#define POISON_PILL "POISON_PILL"

typedef struct
{
    // char* request_file;
    FILE* writer;
    char** threadarg;

    array* arr;

    pthread_mutex_t mutex_read;
    pthread_mutex_t mutex_write;

    int* index;
    int num_files;
} requester;

typedef struct
{
    char* resolve_file;
    FILE* writer;

    array* arr;

    int* index;
    int num_files;
    pthread_mutex_t mutex_read;
} resolver;

void* requesting(void* req);
void* resolving(void* res);

#endif