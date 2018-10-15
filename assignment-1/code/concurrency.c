#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details. 
#include <pthread.h> 
#include <time.h>

struct Package {

    int data;

    double wait; // Some floating point value from 2 to 9
}; 



void * produce(void *vargp) 
{
    struct Package * buffer = vargp;
    int g = 0;
    sleep(rand()  % 3); 
    printf("Creating a new Package\n");
    for(int i = 0; i < 32; i ++){
        if(buffer[i].data != 0)
        g++;
    }
    buffer[g].data = rand() % 1000000 + 1;
    printf("Data %d was produced. Position in line: %d\n", buffer[g].data, g + 1);
    return NULL; 
} 
   
void * consume(void *vargp) 
{
    struct Package * buffer = vargp;
    int g = 0;
    int data;
    sleep(rand()  % 3 + 3); 
    if(buffer[0].data == 0){}
    else{
        printf("Consuming a Package\n");
        data = buffer[0].data;
        for(int i = 0; i < 31; i++){
            buffer[i] = buffer[i + 1];
        }
        buffer[31].data = 0;
        buffer[31].wait = 0.0;
        printf("Data %d was consumed.\n", data);
    }
        return NULL; 
}
 
int main() 
{
    srand(time(NULL)); 
    struct Package * buffer = malloc(32 * sizeof(struct Package));

    for(int i = 0; i < 32; i++){
        buffer[i].data = 0;
        buffer[i].wait = 0.0;
    }

    pthread_t thread_id1, thread_id2, thread_id3, thread_id4; 
    
    pthread_create(&thread_id1, NULL, produce, buffer); 
    pthread_create(&thread_id2, NULL, produce, buffer); 
    pthread_create(&thread_id3, NULL, consume, buffer); 
    pthread_create(&thread_id4, NULL, consume, buffer); 
       
 
    pthread_join(thread_id1, NULL); 
    pthread_join(thread_id2, NULL); 
    pthread_join(thread_id3, NULL); 
    pthread_join(thread_id4, NULL); 
    
    printf("Slot 1: %d\n", buffer[0].data);
    printf("Slot 2: %d\n", buffer[1].data);
    free(buffer);
    exit(0); 
}
