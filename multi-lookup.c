#include "multi-lookup.h"

#include "util.h"
#include "array.h"

#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/time.h>
#include <sys/sem.h>

#define MAX_INPUT_FILES 100
#define MAX_REQUESTER_THREADS 10
#define MAX_RESOLVER_THREADS 10
#define MAX_IP_LENGTH INET6_ADDRSTRLEN

void* requesting(void* req)
{
    //printf("printing");
    //int index = 0;
    bool over = false;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    requester* r = (requester*)req;
    /*pthread_mutex_lock(&r->mutex_write);
        FILE* writer = fopen(r->request_file, "w");
        if (!writer)
        {
            fprintf(stderr, "invalid file: %s", r->request_file);
            return NULL;
        }
    pthread_mutex_unlock(&r->mutex_write);*/
    while(over != true) 
    {
        //printf("in loop again\n");
        ///why is my mutex error
        //printf("Index: %d\n", *(r->index));
        //printf("%d\n", r->num_files);

        if(*(r->index) >= r->num_files)//going to 8 since array size and stack
        {
            //array_put(r->arr, "fin");
            //array_put(r->arr, POISON_PILL);
            gettimeofday(&end, NULL);
            double runnin = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000;
            printf("Thread %lx serviced %d files in %lf \n", pthread_self(), *r->index, runnin);
            over = true;
            return NULL;
        }
        
        pthread_mutex_lock(&r->mutex_read);
        FILE* opener = fopen(r->threadarg[*r->index], "r");
        if (opener == NULL)
        {
            //printf("INDEX: %d", *r->index);
            //printf("Invalid file at index:%d\n %s", index, r->threadarg[*r->index]);
            fprintf(stderr, "invalid file name:%s\n", r->threadarg[*r->index]);
            *(r->index) = *(r->index) + 1;
            pthread_mutex_unlock(&r->mutex_read);

            continue;
            //return NULL;
        }
        *(r->index) = *(r->index) + 1;
        //printf("index val: ");
        pthread_mutex_unlock(&r->mutex_read);
        char hostname[MAX_NAME_LENGTH];
        while (fgets(hostname, sizeof(hostname), opener))
        {
            
            //printf("In loop yurr \n");
            hostname[strlen(hostname) - 1] = '\0';
            //printf("%s\n", hostname);
            array_put(r->arr, hostname);
            pthread_mutex_lock(&r->mutex_read);
            fprintf(r->writer, "%s\n", hostname);
            //printf("Name posted: %s", hostname);
            pthread_mutex_unlock(&r->mutex_read);
            // else
            // {
            //     printf("not fucking writing\n");
            // }
        }
        fclose(opener);
        
    }

    return NULL;
}

void* resolving(void* res)
{
    //printf("in resolving\n");
    bool empty = false;
    struct timeval start, end;
    int count = 0;
    gettimeofday(&start, NULL);
    resolver* r = (resolver*)res;
    /*pthread_mutex_lock(&r->mutex_read);
    FILE* writer = fopen(r->resolve_file, "w");
    if (!writer)
    {
        //printf("res writer not open\n");
        fprintf(stderr, "invalid file name: %s", r->resolve_file);
        return NULL;
    }
    pthread_mutex_unlock(&r->mutex_read);*/
    char* hostname;
    char ip[MAX_IP_LENGTH];
    while(empty != true)
    {
        if(*(r->index) >= r->num_files && r->arr->top == 0)//going to 8 since array size and stack
        {
            //array_put(r->arr, POISON_PILL);
            gettimeofday(&end, NULL);
            double runnin = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000;
            printf("Thread %lx serviced %d files in %lf \n", pthread_self(), *r->index, runnin);
            empty = true;
            return NULL;
        }
        /*if (r->index == num_files)
        {
            gettimeofday(&end, NULL);
            double runnin = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000;
            printf("Thread %lx serviced %d hostnames in %lf \n", pthread_self(), count, runnin);
            empty = true;
            //free(hostname);
            //free(ip);
            return NULL;
        }*/
        //pthread_mutex_lock(&r->mutex);
        //printf("making hostname\n");
        array_get(r->arr, &hostname);//stall
        //printf("hostname is: %s", hostname);
        int s = dnslookup(hostname, ip, MAX_IP_LENGTH);
        pthread_mutex_lock(&r->mutex_read);
        if (s == 0)
        {
            fprintf(r->writer, "%s, %s\n", hostname, ip);
            //printf("currently printing: %s and %s\n", hostname, ip);
            count++;
        }
        else
        {
            fprintf(r->writer, "%s, NOT_RESOLVED\n", hostname);
        }
        pthread_mutex_unlock(&r->mutex_read);
        /*if (strcmp(hostname, "fin") == 0)
        {
            printf("resolving over\n");
            gettimeofday(&end, NULL);
            double runnin = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000;
            printf("Thread %lx serviced %d hostnames in %lf \n", pthread_self(), count, runnin);
            empty = true;
            fclose(writer);
            free(hostname);
            free(ip);
            pthread_mutex_unlock(&r->mutex);
            return NULL;
        }*/
        //pthread_mutex_unlock(&r->mutex);
    }
    //fclose(writer);
    //free(hostname);
    //free(ip);
    return NULL;
}

int main(int argc, char* argv[])
{
    if (argc > MAX_INPUT_FILES + 5)
    {
        fprintf(stderr, "to use this program: ./multi-lookup <num_requesters> <num_resolvers> <request output file> <resolve output file> <input1.txt> ... <input99.txt>");
        return -1;
    }
    else if (argc < 6)
    {
        fprintf(stdout, "Need more arguments: ./multi-lookup <num_requesters> <num_resolvers> <request output file> <resolve output file> <input1.txt>");
        return -1;
    }

    int request_num = atoi(argv[1]);
    int resolve_num = atoi(argv[2]);

    if (request_num > MAX_REQUESTER_THREADS || resolve_num > MAX_RESOLVER_THREADS || request_num < 1 || resolve_num < 1)
    {
        //printf("Request or Resolve Thread Count Missed");
        fprintf(stderr, "Request or Resolve Thread Count Missed");
        return -1;
    }

    requester req;
    resolver res;
    pthread_t req_thread[request_num];
    pthread_t res_thread[resolve_num];

    pthread_mutex_t mutex_req;
    pthread_mutex_init(&mutex_req, NULL);
    pthread_mutex_t mutex_res;
    pthread_mutex_t mutex_read;
    pthread_mutex_init(&mutex_res, NULL);
    pthread_mutex_init(&mutex_read, NULL);


    array share_arr;
    array_init(&share_arr);    

    int index = 0;

    //array_init(&req.arr);
    // req.request_file = argv[3];
    req.arr = &share_arr;
    req.num_files = argc - 5;
    req.index = &index;
    req.mutex_read = mutex_read;
    req.mutex_write = mutex_req;
    req.writer = fopen(argv[3], "w");

    res.resolve_file = argv[4];
    res.arr = &share_arr;
    res.mutex_read = mutex_res;
    res.index = &index;
    res.num_files = argc-5;
    res.writer = fopen(res.resolve_file, "w");

    // req.threadarg = (char**)malloc(request_num*sizeof(char*));
    // for (int j = 0; j < req.num_files; j++)
    // {
    //     req.threadarg[j] = (char*)malloc(MAX_NAME_LENGTH*sizeof(char));
    //     req.threadarg[j] = argv[j + 5];
    //     //printf("arg is %s", req.threadarg[j]);
    // }

    req.threadarg = &argv[5];

    for (int i = 0; i < request_num; i++)//make this into 1
    {
        if(pthread_create(&req_thread[i], NULL, &requesting, (void*)&req))
        {
            printf("error!\n");
            return 1;
        }
    }

    for (int i = 0; i < resolve_num; i++)
    {
        if (pthread_create(&res_thread[i], NULL, &resolving, (void*)&res))
        {
            printf("error on resolve create\n");
            return 1;
        }
    }

    for (int i = 0; i < request_num; i++)
    {
        if(pthread_join(req_thread[i], NULL))
        {
            printf("error 2\n");
            return 3;
        }
    }

    for (int i = 0; i < resolve_num; i++)
    {
        if (pthread_join(res_thread[i], NULL))
        {
            printf("error on join\n");
            return 3;
        }
    }

    /*for (int j = 0; j < request_num; j++)
    {
        free(req.threadarg[j]);
    }*/
    // free(req.threadarg);
    array_free(req.arr);
    pthread_mutex_destroy(&mutex_read);
    pthread_mutex_destroy(&mutex_res);
    pthread_mutex_destroy(&mutex_req);
    fclose(res.writer);


    //sem_init(&(res.resolve_lock), 0, 1);
    //pthread_mutex_init(&res.mutex, NULL);

    //pthread_t res_thread[resolve_num];

//error

    //returninstead of pthread exit
    //fclose()
    
}