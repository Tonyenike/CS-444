#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details. 
#include <pthread.h> 


struct Package {

    int data;

    double wait; // Some floating point value from 2 to 9
}; 



void * myThreadFun(void *vargp) 
{ 
    sleep(1); 
    printf("Creating a new Package\n");
    // length(buffer); 
    return NULL; 
} 
   
int main() 
{ 
    struct Package * buffer = malloc(32 * sizeof(struct Package));

    pthread_t thread_id; 
    printf("Before Thread\n"); 
    pthread_create(&thread_id, NULL, myThreadFun, NULL); 
    pthread_join(thread_id, NULL); 
    printf("After Thread\n"); 
    exit(0); 
}
