#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details. 
#include <pthread.h> 
#include <time.h>

#include "mt.c"


#define MAX_USERS 3

pthread_mutex_t lock;


int user_count = 0;
int shutdown = 0;

// Returns 1 if we can use testrand, 
// Returns 0 if we have to use MT.


// This code is from the resources folder on canvas on the CS 444 class page. 
// It checks to see if we can use rdrand
int testrand(){
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    
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




void * use_resource(void * vargp) 
{
    int yeet = (int) vargp;
    while(1){
    int wait = getrandint(1, 40);
    sleep(wait);

    int stop = 0;
    int mess = 0;

    while(!stop){
        sleep(.1);
        pthread_mutex_lock(&lock); 
        if(!shutdown){
            user_count++;
            stop = 1;
            if(user_count == MAX_USERS){
                shutdown = 1;
            }
        }
        else if(mess == 0){
            printf("PROCESS %d: Waiting to use resource...\n",yeet);
            mess = 1;
        }
        pthread_mutex_unlock(&lock);
    }


    printf("PROCESS %d: Using resource...\n", yeet);
    wait = getrandint(5,12); 
    sleep(wait);    

    
    printf("PROCESS %d: Done with resource.\n", yeet);
   


 
    pthread_mutex_lock(&lock); 
        user_count--;
        if(user_count == 0)
            shutdown = 0;
    pthread_mutex_unlock(&lock);
    
    yeet = yeet + 10;
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

    for(int i = 0; i < 100; i ++){
    printf("\n");
    }
    printf("Welcome to assignment 3, problem 1. Press CTRL + C to exit\n\n");

    pthread_t thread_id1, thread_id2, thread_id3, thread_id4, thread_id5, thread_id6, thread_id7, thread_id8, thread_id9, thread_id10; 

    //Create threads
    pthread_create(&thread_id1, NULL, use_resource, 1); 
    pthread_create(&thread_id2, NULL, use_resource, 2); 
    pthread_create(&thread_id3, NULL, use_resource, 3); 
    pthread_create(&thread_id4, NULL, use_resource, 4); 
    pthread_create(&thread_id5, NULL, use_resource, 5); 
    pthread_create(&thread_id6, NULL, use_resource, 6); 
    pthread_create(&thread_id7, NULL, use_resource, 7); 
    pthread_create(&thread_id8, NULL, use_resource, 8); 
    pthread_create(&thread_id9, NULL, use_resource, 9); 
    pthread_create(&thread_id10, NULL, use_resource, 10); 
       
    // Join threads
    pthread_join(thread_id1, NULL); 
    pthread_join(thread_id2, NULL); 
    pthread_join(thread_id3, NULL); 
    pthread_join(thread_id4, NULL); 
    pthread_join(thread_id5, NULL); 
    pthread_join(thread_id6, NULL); 
    pthread_join(thread_id7, NULL); 
    pthread_join(thread_id8, NULL); 
    pthread_join(thread_id9, NULL); 
    pthread_join(thread_id10, NULL); 
    
    exit(0); 
}
