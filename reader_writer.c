/*
Sample Output:
Writer: Written global value: 1. Readers: 0. Writers: 1.
Writer: Written global value: 2. Readers: 0. Writers: 1.
Writer: Written global value: 3. Readers: 0. Writers: 1.
Reader: Read global value: 3. Readers: 5. Writers: 0.
Reader: Read global value: 3. Readers: 4. Writers: 0.
Reader: Read global value: 3. Readers: 3. Writers: 0.
Reader: Read global value: 3. Readers: 2. Writers: 0.
Reader: Read global value: 3. Readers: 1. Writers: 0.
Writer: Written global value: 4. Readers: 0. Writers: 1.
Writer: Written global value: 5. Readers: 0. Writers: 1.
Parent(/main) quiting
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_WRITERS 5
#define NUM_READERS 5

int global_variable = 0;
int num_writers = 0;
int num_readers = 0;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;  	/* mutex lock for shared variable */
pthread_cond_t c_write = PTHREAD_COND_INITIALIZER; /* writers waits on this cond var */
pthread_cond_t c_read = PTHREAD_COND_INITIALIZER; /* readers waits on this cond var */

void *writer_procedure (void *param);
void *reader_procedure (void *param);

int main(int argc, char *argv[]) {

	pthread_t writer[NUM_WRITERS], reader[NUM_READERS];

    for(int i=0; i<NUM_WRITERS; i++){
        // Create the writer thread.
        if(pthread_create(&writer[i], NULL, writer_procedure, NULL) != 0) {
            fprintf(stderr, "Unable to create writer thread\n");
            exit(1);
        }
    }
    for(int i=0; i<NUM_READERS; i++){
        // Create the reader threads.
        if(pthread_create(&reader[i], NULL, reader_procedure, NULL) != 0) {
            fprintf(stderr, "Unable to create reader thread\n");
            exit(1);
        }
    }

    // Wait for exit of threads created because we are using default
    // joinable property of threads. Not doing this will result in 
    // zombies threads.
    for(int i=0; i<NUM_WRITERS; i++){
        // Wait for the writer thread to exit.
        pthread_join(writer[i],NULL);
    }
    for(int i=0; i<NUM_READERS; i++){
        // Wait for the reader thread to exit.
        pthread_join(reader[i],NULL);
    }

	printf("Parent(/main) quiting\n");

	return 0;
}

/* Write value(s) */
void *writer_procedure(void *param) {
    pthread_mutex_lock(&m);
    while(num_readers>0 || num_writers>0){
        pthread_cond_wait(&c_write, &m);
    }
    num_writers++;
    pthread_mutex_unlock(&m);
    sleep(1);
    global_variable+=1;
    printf("Writer: Written global value: %d. "
            "Readers: %d. Writers: %d.\n",
            global_variable, num_readers, num_writers);
    pthread_mutex_lock(&m);
    num_writers--;
    pthread_mutex_unlock(&m);
    pthread_cond_broadcast(&c_write);
    pthread_cond_broadcast(&c_read);
    return 0;
}

/* Read value(s); Note the consumer never terminates */
void *reader_procedure(void *param) {
    pthread_mutex_lock(&m);
    while(num_writers>0){
        pthread_cond_wait(&c_read, &m);
    }
    num_readers++;
    pthread_mutex_unlock(&m);
    sleep(1);
    printf("Reader: Read global value: %d. "
            "Readers: %d. Writers: %d.\n",
            global_variable, num_readers, num_writers);
    pthread_mutex_lock(&m);
    num_readers--;
    pthread_mutex_unlock(&m);
    pthread_cond_broadcast(&c_write);
    return 0;
}
