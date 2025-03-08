#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#include "combiner.h"
#include "hashmap.c"
#include "mapper.c"
#include "reducer.c"


int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("Too few arguments\n");
        return -1;
    }

    num_slots = atoi(argv[1]);
    num_users = atoi(argv[2]);
    if (num_slots < 1 || num_users < 1) {
        printf("Invalid options");
        return -1;
    }

    comm_buf = mmap(NULL, num_users * sizeof(comm_buf_t), PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
    tuple_t *tuple_buf_block = mmap(NULL, num_users * sizeof(tuple_t) * num_slots, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);

    pthread_mutexattr_t pshared_mtx_attr;
    pthread_condattr_t pshared_cond_attr;
    pthread_mutexattr_init(&pshared_mtx_attr);
    pthread_mutexattr_setpshared(&pshared_mtx_attr, PTHREAD_PROCESS_SHARED);
    pthread_condattr_init(&pshared_cond_attr);
    pthread_condattr_setpshared(&pshared_cond_attr, PTHREAD_PROCESS_SHARED);

    for (int i = 0; i < num_users; i++) {
        pthread_cond_init(&comm_buf[i].empty, &pshared_cond_attr);
        pthread_cond_init(&comm_buf[i].empty, &pshared_cond_attr);
        comm_buf[i].tuple_buf = &tuple_buf_block[i * num_slots]; 
        if (comm_buf[i].tuple_buf == NULL){
            printf("Failure to allocate space for buffer\n");
            return -1;
        }
        pthread_mutex_init(&comm_buf[i].mutex, &pshared_mtx_attr);
        comm_buf[i].in_buf_loc = 0;
        comm_buf[i].taken = 0;
        comm_buf[i].fill = 0;
        comm_buf[i].out_buf_loc = 0;
	    strcpy(comm_buf[i].userID, "x");

    }

    pid_t *forkids = malloc(num_users * sizeof(pid_t));

    for (int i = 0; i < num_users; i++) {
        forkids[i] = fork();

        if (forkids[i] < 0) {

            printf("Failed fork on child %d\n", i);

        } else if (forkids[i] == 0) {

            reducer(i);
            exit(0);

        } else {
            
            continue;

        }
    }

    mapper();

    int i = 0;
    while (i < num_users) {

        int err = wait(NULL);
        if (err == -1) {
            continue;
        }
        i++;
    }
    
    return 0;
    
}
