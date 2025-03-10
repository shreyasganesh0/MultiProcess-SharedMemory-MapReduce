void reducer(int argc){
    int idx = argc;

    comm_buf_t *curr_buf = &comm_buf[idx];
    curr_buf->topic_score_map = create_hashmap(INITIAL_CAPACITY); //can be process local

    do{
        pthread_mutex_lock(&curr_buf->mutex);
        while (curr_buf->fill == 0) {
            pthread_cond_wait(&curr_buf->full, &curr_buf->mutex); 
        }

        int run_length = 0;
        while(curr_buf->in_buf_loc != curr_buf->out_buf_loc){

            tuple_t *curr_tup = &curr_buf->tuple_buf[curr_buf->out_buf_loc];
            if (!strcmp(curr_tup->topic,"xeof")){ 
                hashmap_iterate(curr_buf->topic_score_map, curr_buf->userID);
                pthread_mutex_unlock(&curr_buf->mutex); //maybe not needed?
                return;
            }
            else{
                hashmap_insert(curr_buf->topic_score_map, curr_tup->topic, curr_tup->score);
            }
            curr_buf->out_buf_loc++;
            curr_buf->out_buf_loc = curr_buf->out_buf_loc - (curr_buf->out_buf_loc >= num_slots) * num_slots;
            curr_buf->fill--;
            run_length++;
        }

        pthread_mutex_unlock(&curr_buf->mutex);
        for (int i = 0; i < run_length; i++) {
		pthread_cond_signal(&curr_buf->empty);
        }
        
    } while(1);

    return;
}
