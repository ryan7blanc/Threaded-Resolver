#include "array.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

int array_init(array *s)
{
    sem_init(&(s->empty), 0, ARRAY_SIZE);
    sem_init(&(s->full), 0, 0);
    sem_init(&(s->mutex), 0, 1);

    s->top = 0;
    s->arr = (char**)malloc(sizeof(char*) * ARRAY_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        s->arr[i] = (char*)malloc(sizeof(char) * MAX_NAME_LENGTH);
    }
    return 0;
}

int array_put(array *s, char *hostname)
{
    sem_wait(&(s->empty));
    sem_wait(&(s->mutex));
        strcpy(s->arr[s->top++], hostname);
        //s->top++;
    sem_post(&(s->mutex));
    sem_post(&(s->full));
    return 0;
}

int array_get(array *s, char **hostname)
{
    sem_wait(&(s->full));
    sem_wait(&(s->mutex));
        //s->top--;
    *hostname = malloc(MAX_NAME_LENGTH);
    strcpy(*hostname, s->arr[--s->top]);
    //free(hostname);
    sem_post(&(s->mutex));
    sem_post(&(s->empty));
    return 0;
}

void array_free(array *s)
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        free(s->arr[i]);
    }
    free(s->arr);
    
    sem_destroy(&(s->mutex));
    sem_destroy(&(s->full));
    sem_destroy(&(s->empty));
}