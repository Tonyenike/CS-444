#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details. 
#include <pthread.h> 
#include <time.h>

#include "mt.c"

int buffersize = 32; // You may change this as you wish.



pthread_mutex_t lock;




struct Package {

    int data; // Some "data" being processed

    int wait; // Some sleep time from 2 to 9 seconds
}; 


// Returns 1 if we can use testrand, 
// Returns 0 if we have to use MT.


// This code is from the resources folder on canvas on the CS 444 class page. 
// It checks to see if we can use rdrand
int testrand(){
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;

    char vendor[13];
    
    eax = 0x01;

    __asm__ __volatile__(
                         "cpuid;"
                         : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                         : "a"(eax)
                         );
    
    if(ecx & 0x40000000){
        return 1;
    }
    else{
        return 0;
    }
}

// Gets a random int from bot to top inclusive.
int getrandint(int bot, int top){

    int doy = 0;
    int result = 0;

    if(!testrand()){
        doy = (int) genrand_int31();
    }
    else{
        __asm__ __volatile__("rdrand %0":"=r"(doy));
    }

    result = bot + (doy % (top - bot));

    return result;
}




void * produce(void *vargp) 
{

    while(1){
    int wait = getrandint(3, 7);
    sleep(wait);



    pthread_mutex_lock(&lock); 
    struct Package * buffer = vargp;

    int g = 0;
    // printf("%d\n", wait); // For debugging purposes.
    for(int i = 0; i < buffersize; i ++){
        if(buffer[i].data != 0)
        g++;
    }
    if(g != buffersize){
        buffer[g].wait = getrandint(2, 9);
        buffer[g].data = getrandint(1, 1000);
        printf("PRODUCER: Data %d was produced. Total items in queue: %d\n", buffer[g].data, g + 1);
    }
    pthread_mutex_unlock(&lock);
    }
    return NULL; 
} 


void * consume(void *vargp) 
{
    while(1){
    int data;
    int wait;
    pthread_mutex_lock(&lock);
    struct Package * buffer = vargp;
    // Check if the buffer has an item
    if(buffer[0].data != 0){
        data = buffer[0].data;
        wait = buffer[0].wait;
        for(int i = 0; i < buffersize - 1; i++){
            buffer[i] = buffer[i + 1];
        }
        buffer[buffersize - 1].data = 0;
        buffer[buffersize - 1].wait = 0;
        
        printf("CONSUMER: Data %d was taken off queue. Consuming... \n", data);
        pthread_mutex_unlock(&lock);
        
        sleep(wait);
    }
    else{
        pthread_mutex_unlock(&lock);
    }
    }
        return NULL; 
}



int main() 
{
    pthread_mutex_init ( &lock, NULL); 

    // unsigned long init[4] = {0x123, 0x234, 0x345, 0x456};
    // unsigned long length = 4;
    // init_by_array(init, length);
    // init_genrand(time(NULL));

    struct Package * buffer = malloc(buffersize * sizeof(struct Package));

    for(int i = 0; i < buffersize; i++){
        buffer[i].data = 0;
        buffer[i].wait = 0.0;
    }

    for(int i = 0; i < 100; i ++){
    printf("\n");
    }
    printf("Welcome to producer / consumer: assignment 1. Press CTRL + C to exit\n\n");

    pthread_t thread_id1, thread_id2, thread_id3, thread_id4, thread_id66; 
    
    //Create producers
    pthread_create(&thread_id1, NULL, produce, buffer); 
    pthread_create(&thread_id2, NULL, produce, buffer); 
    pthread_create(&thread_id66, NULL, produce, buffer); 
    //Create consumers
    pthread_create(&thread_id3, NULL, consume, buffer); 
    pthread_create(&thread_id4, NULL, consume, buffer); 
       
    // Join threads
    pthread_join(thread_id1, NULL); 
    pthread_join(thread_id2, NULL); 
    pthread_join(thread_id66, NULL); 
    pthread_join(thread_id3, NULL); 
    pthread_join(thread_id4, NULL); 
    
    free(buffer);
    exit(0); 
}
