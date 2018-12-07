/*
 * Group 7, Concurrency 3, CS 444
 *
 * Benjamin Martin and Nickoli Londura
 *
 * martinb3@oregonstate.edu
 * londuran@oregonstate.edu
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details. 
#include <pthread.h> 
#include <time.h>

#include "mt.h"
#include "link.h"
#include "stack.h"


pthread_mutex_t lock;

struct Table{
    int inserter_in_use;
    int deleter_in_use;
    int deleter_in_queue;
    int scanners;
};

struct Table tpose;


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
    
    if(top - bot == 0)
        result = bot;
    else
        result = bot + (doy % (top - bot));

    return result;
}


/*
 * Returns the value of the element removed if successful, returns -1 if failure (position doesn't exist)
 *
 * Position specifies the position of the element to be removed. Position = 1 refers to the topmost element of the stack.
 * 
 */

int delete(struct stack * stack){

    int val;   
 
    int yeet = 0;
    int cheeto = 0;
    while(!yeet){
        pthread_mutex_lock(&lock);
        if(tpose.inserter_in_use == 0 && tpose.scanners == 0 && tpose.deleter_in_use == 0){
            tpose.deleter_in_use = 1;
            pthread_mutex_unlock(&lock);
            printf("DELETER: deleting...\n");
            yeet = 1;
        }
        else{
            tpose.deleter_in_queue = 1;
            pthread_mutex_unlock(&lock);
            if(!cheeto){
                printf("DELETER: Waiting to access resource.\n");
                cheeto = 1;
            }
            sleep(.5); 
        }
    }
    sleep(1); 
            printf("DELETER: deleting...\n");
    sleep(1); 
            printf("DELETER: deleting...\n");
    sleep(1); 
            printf("DELETER: deleting...\n");
    sleep(1); 
            printf("DELETER: deleting...\n");
    sleep(1); 
    int jetus = stack_size(stack);
    if(jetus != 0){
        int position = getrandint(1, jetus);
        val = remove_by_index(stack, position); 
        printf("DELETER: removed Item %d from stack, which had value: %d\n", position, val);
        print_contents(stack);
    }
    else{
        printf("DELETER: Stack was empty so I left!\n");
    }
  
    pthread_mutex_lock(&lock);
    tpose.deleter_in_queue = 0;
    tpose.deleter_in_use = 0; 
    pthread_mutex_unlock(&lock);

    return val;
}

/*
 * Pushes value to the TOP of the stack. Has no return value.
 */

void insert(struct stack * stack, int value){

    
    int yeet = 0;
    int cheeto = 0;
    while(!yeet){
        pthread_mutex_lock(&lock);
        if(tpose.inserter_in_use == 0 && tpose.deleter_in_queue == 0 && tpose.deleter_in_use == 0){
            tpose.inserter_in_use = 1;
            pthread_mutex_unlock(&lock);
            yeet = 1;
        }
        else{
            pthread_mutex_unlock(&lock);
            if(!cheeto){
                printf("INSERTER: Waiting to access resource.\n");
                cheeto = 1;
            }
            sleep(.5); 
        }
    }

    
    stack_push(stack, value);
    sleep(3);
    printf("INSERTER: pushed %d to stack\n", value);
    print_contents(stack);
    
    pthread_mutex_lock(&lock); 
        tpose.inserter_in_use = 0;
    pthread_mutex_unlock(&lock); 

    return;
}

/*
 * Returns 1 if the value exists on the stack, returns 0 otherwise.
 */
int scan(struct stack * stack, int value){

    int cheeto = 0;
    int yeet = 0;
    while(!yeet){
        pthread_mutex_lock(&lock);
        if(tpose.deleter_in_queue == 0 && tpose.deleter_in_use == 0){
            tpose.scanners++;
            pthread_mutex_unlock(&lock);
            yeet = 1;
        }
        else{
            pthread_mutex_unlock(&lock);
            if(!cheeto){
                printf("SCANNER: Waiting to access resource.\n");
                cheeto = 1;
            }
            sleep(.5); 
        }
    }

    sleep(1);
    int monies = scan_stack(stack, value);
    if(monies){
        printf("SCANNER: found value %d in the stack\n", value);
        pthread_mutex_lock(&lock);
            tpose.scanners--;
        pthread_mutex_unlock(&lock);
        return monies;
    }
    else{
        printf("SCANNER: Value %d is not in the stack\n", value);
        pthread_mutex_lock(&lock);
            tpose.scanners--;
        pthread_mutex_unlock(&lock);
        return 0;
    }
}

void * inserter(void * vargp)
{
    struct stack * stack =  (struct stack *) vargp;

    while(1){
        sleep(getrandint(5, 10));
        insert(stack, getrandint(1, 100));
    }

    return NULL;
}

void * scanner(void * vargp)
{

    struct stack * stack =  (struct stack *) vargp;

    while(1){
        sleep(getrandint(5, 10));
        scan(stack, getrandint(1, 100));
    }

    return NULL;

}


void * deleter(void * vargp)
{

    struct stack * stack =  (struct stack *) vargp;

    while(1){
        sleep(getrandint(15, 45));
        delete(stack);
    }

    return NULL;

}

int main(int argc, char** argv) {


    tpose.inserter_in_use = 0;
    tpose.deleter_in_use = 0;
    tpose.deleter_in_queue = 0;

    struct stack * stack = stack_create();
    pthread_mutex_init ( &lock, NULL); 
   
    for(int i = 0; i < 100; i ++){
    printf("\n");
    }
    printf("Welcome to assignment 3, problem 2. Press CTRL + C to exit\n\n");

    pthread_t thread_id1; 
    pthread_t thread_id2; 
    pthread_t thread_id3; 
    pthread_t thread_id4; 
    pthread_t thread_id5; 
    pthread_t thread_id6; 
    pthread_t thread_id7; 
    pthread_t thread_id8; 

    //Create threads
    pthread_create(&thread_id1, NULL, inserter, (void *) stack); 
    pthread_create(&thread_id2, NULL, inserter, (void *) stack); 
    pthread_create(&thread_id3, NULL, scanner, (void *) stack); 
    pthread_create(&thread_id4, NULL, scanner, (void *) stack); 
    pthread_create(&thread_id5, NULL, scanner, (void *) stack); 
    pthread_create(&thread_id6, NULL, scanner, (void *) stack); 
    pthread_create(&thread_id7, NULL, deleter, (void *) stack); 
    pthread_create(&thread_id8, NULL, deleter, (void *) stack); 
       
    // Join threads
    pthread_join(thread_id1, NULL); 
    pthread_join(thread_id2, NULL); 
    pthread_join(thread_id3, NULL); 
    pthread_join(thread_id4, NULL); 
    pthread_join(thread_id5, NULL); 
    pthread_join(thread_id6, NULL); 
    pthread_join(thread_id7, NULL); 
    pthread_join(thread_id8, NULL); 

    stack_free(stack);

    return 0;
}
