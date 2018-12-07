#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details. 
#include <pthread.h> 
#include <time.h>

#include "mt.c"

pthread_mutex_t forks;

pthread_mutex_t pstatus;

int status[5] = {0,  // Status of fork 1
                 0,  // Status of fork 2
                 0,  // Status of fork 3
                 0,  // Status of fork 4
                 0}; // Status of fork 5

int pstatust[5] = {  0,
                    0,
                    0,
                    0,
                    0};

char * names[5] = { "Aristotle",
                    "Baruch Spinoza",
                    "Confucious",
                    "David Hume",
                    "Epicurus"};

char history[65536] = "";


struct Philosopher {

    char * name;
    int forkleft;
    int forkright;
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

    unsigned int doy = 0;
    unsigned int result = 0;

    if(!testrand()){
        doy = (int) genrand_int31();
    }
    else{
        __asm__ __volatile__("rdrand %0":"=r"(doy));
    }

    result = bot + (doy % (top - bot));

    return result;
}

char * getstatus(int num){

    if(num == 0)
        return "Thinking";
    else if(num == 1)
        return "Eating";
    else if(num == 2)
        return "Waiting for forks";

}

void printtable(){


for(int i = 0; i < 100; i++){
    printf("\n");
}

printf("%s", history);

printf("---------------------------------\n");
printf("%s: %s\n", names[0], getstatus(pstatust[0]));
printf("%s: %s\n", names[1], getstatus(pstatust[1]));
printf("%s: %s\n", names[2], getstatus(pstatust[2]));
printf("%s: %s\n", names[3], getstatus(pstatust[3]));
printf("%s: %s\n", names[4], getstatus(pstatust[4]));
printf("---------------------------------\n");

}

void think(char * name, int forkleft)
{
    char buffer[128];
    int wait = getrandint(1, 20);
    // printf("%s is thinking for %d seconds\n", name, wait);
    snprintf(buffer, sizeof(buffer), "%s is thinking for %d seconds\n", name, wait);
    pthread_mutex_lock(&pstatus);
    strcat(history, buffer);
    pstatust[forkleft] = 0;
    printtable();
    pthread_mutex_unlock(&pstatus);



    sleep(wait);
}

void eat(char * name, int forkleft)
{
    char buffer[128];
    int wait = getrandint(2, 9);
    // printf("%s is eating for %d seconds\n", name, wait);
    snprintf(buffer, sizeof(buffer), "%s is eating for %d seconds\n", name, wait);
    pthread_mutex_lock(&pstatus);
    strcat(history, buffer);
    pstatust[forkleft] = 1;
    printtable();
    pthread_mutex_unlock(&pstatus);


    sleep(wait);
}


void get_forks(char * name, int forkleft, int forkright)
{
    int printed = 0;
    int gotem = 0;
    while(!gotem)
    {


    pthread_mutex_lock(&forks);
    
    if(status[forkleft] == 0 && status[forkright] == 0){
        status[forkleft] = 1;
        status[forkright] = 1;
        pthread_mutex_unlock(&forks);
        gotem = 1;
    }
    else{
        pthread_mutex_unlock(&forks);
        if(!printed){
            char buffer[128];
            // pthread_mutex_lock(&pstatus);
            // printf("%s is waiting for his forks\n", name);
            snprintf(buffer, sizeof(buffer), "%s is waiting for forks\n", name);
            pthread_mutex_lock(&pstatus);
            strcat(history, buffer);
            pstatust[forkleft] = 2;
            printtable();
            pthread_mutex_unlock(&pstatus);
            printed = 1;
        }
    }
    }
}

void put_forks(char * name, int forkleft, int forkright)
{

    pthread_mutex_lock(&forks);
        status[forkleft] = 0;
        status[forkright] = 0;
    pthread_mutex_unlock(&forks);
}

void * philosopher(void *vargp) 
{
    struct Philosopher  phil =  * (struct Philosopher *) vargp;
    while(1) {
	    
       think(phil.name, phil.forkleft);
	   get_forks(phil.name, phil.forkleft, phil.forkright);
	   eat(phil.name, phil.forkleft);
	   put_forks(phil.name, phil.forkleft, phil.forkright);

    }
}

int main() 
{

    pthread_mutex_init ( &forks, NULL); // Fork status manager
 
   pthread_mutex_init ( &pstatus, NULL); // Fork 5

    // unsigned long init[4] = {0x123, 0x234, 0x345, 0x456};
    // unsigned long length = 4;
    // init_by_array(init, length);
    // init_genrand(time(NULL));


    struct Philosopher *  philosophers =  malloc(5 * sizeof(struct Philosopher));
    philosophers[0].name = names[0];
    philosophers[1].name = names[1];
    philosophers[2].name = names[2];
    philosophers[3].name = names[3];
    philosophers[4].name = names[4];


    for(int i = 0; i < 5; i++){
        philosophers[i].forkleft = i;
        philosophers[i].forkright = (i + 1)% 5;
    }


    for(int i = 0; i < 100; i ++){
    	printf("\n");
    }

    printf("Welcome to Dining Philosophers: assignment 2. Press CTRL + C to exit\n\n");

    for(int i = 0; i < 5; i ++){
        printf("%s uses forks %d and %d\n", philosophers[i].name, philosophers[i].forkleft + 1, philosophers[i].forkright + 1);
    }

    sleep(10);

    for(int i = 0; i < 5; i ++){
        printf("\n");
    }

    pthread_t thread_id1, thread_id2, thread_id3, thread_id4, thread_id5; 
    
    //Create philosphers
    pthread_create(&thread_id1, NULL, philosopher, &philosophers[0]); 
    pthread_create(&thread_id2, NULL, philosopher, &philosophers[1]); 
    pthread_create(&thread_id3, NULL, philosopher, &philosophers[2]);     
    pthread_create(&thread_id4, NULL, philosopher, &philosophers[3]);
    pthread_create(&thread_id5, NULL, philosopher, &philosophers[4]); 
       
    // Join threads
    pthread_join(thread_id1, NULL); 
    pthread_join(thread_id2, NULL); 
    pthread_join(thread_id3, NULL); 
    pthread_join(thread_id4, NULL); 
    pthread_join(thread_id5, NULL); 
    
    exit(0); 
}
