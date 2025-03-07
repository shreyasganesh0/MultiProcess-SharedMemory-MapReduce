#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

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

    comm_buf = (comm_buf_t *)malloc(num_users * sizeof(comm_buf_t));

    int forkids = malloc(num_users * sizeof(int));

    for (int i = 0; i < num_users; i++) {
        forkids[i] = fork();

        if (forkids[i] < 0) {

            printf("Failed fork on child %d\n", i);

        } else if (forkids[i] == 0) {

            reducer(i);
            break;

        } else {
            
            continue;

        }
    }
    

    for (int i = 0; i < num_users; i++) {
        sem_init(&comm_buf[i].empty, 0, num_slots);
        sem_init(&comm_buf[i].full, 0, 0);
        comm_buf[i].tuple_buf = (tuple_t *)calloc(num_slots, sizeof(tuple_t));
        if (comm_buf[i].tuple_buf == NULL){
            printf("Failure to allocate space for buffer\n");
            return -1;
        }
        pthread_mutex_init(&comm_buf[i].mutex, NULL);
        comm_buf[i].in_buf_loc = 0;
        comm_buf[i].taken = 0;
        comm_buf[i].out_buf_loc = 0;
        comm_buf[i].topic_score_map = create_hashmap(INITIAL_CAPACITY);
	    strcpy(comm_buf[i].userID, "x");

        red_idxs[i] = i;
        int pred_err = pthread_create(&reducer_tids[i], NULL, reducer, &red_idxs[i]);
        if (pred_err != 0) {
            printf("Thread creation failure\n");
            return -1;
        }
    }


    return 0;
    
}
