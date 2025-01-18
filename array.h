#ifndef ARRAY_H
#define ARRAY_H

#define ARRAY_SIZE 8
#define MAX_NAME_LENGTH 17

#include <semaphore.h>

typedef struct {
    char** arr;
    int top;
    
    sem_t mutex;
    sem_t empty;
    sem_t full;
} array;

int array_init(array *s);
int array_put(array *s, char *hostname);
int array_get(array *s, char **hostname);
void array_free(array *s);

#endif