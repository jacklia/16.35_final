#include "musicretrieval.h"
#include "robot.h"
#include <stdlib.h>
//#include "client.h"
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

music_retrieval * create_music_retrieval(){
    // initialization
    music_retrieval * music = malloc(sizeof(music_retrieval));
    music->run = &run;
    music->music_retrieval_convert = &music_retrieval_convert;
    music->num_robots = 0;
    music->num_notes = 0;
    music->current_note_idx = 0;
    music->robots_updated = 0;
    music->note_array = malloc(num_notes * sizeof(int));
    music->current_note = music->note_array[0];
    music->robots = NULL;
    music->added = 0;
    music ->note_to_play = 8;
    // initialize your synchronization variables here!
    music->note_array_lock = NULL;
    music->new_note_added = NULL;
    music->robot_lock = NULL;
    music->robot_cond = NULL;
    return music;
}
void music_retrieval_convert(struct t_music_retrieval * m, int musical_note){
// music->note_to_play = x

}
void run(struct t_music_retrieval * music){
    while (music->current_note_idx <= music->num_notes) {
        music->music_retrieval_convert(music,music->current_note)
        music->added+=1;
        pthread_cond_broadcast(music->new_note_added);
        // wait on condition variable here
        pthread_mutex_lock(music->note_array_lock);                                                                   //Lock condition
        while (music -> robots_updated != music -> num_robots) {                                              //When not all vehicles are updated
            pthread_cond_wait(music->new_note_added, music->note_array_lock);                                                  //The simulator thread will wait until 
        }                                                                                                   //signaled that new vehicle updated
        pthread_mutex_unlock(music->note_array_lock);                                                                 //Unlock
        // we're past the condition (everyone is updated) so we can now broadcast to the waiting threads
        pthread_mutex_lock(music->note_array_lock);                                                                   //Lock condition
        music->robots_updated = 0;
        music->current_note_idx += 1;
        music->current+note = music->note_array[current_note_idx];
        music->state += 1;
        pthread_cond_broadcast(music->new_note_added);                                                              //Wake up vehicle threads so vehicle_run loop can repeat
        pthread_mutex_unlock(music->note_array_lock);                                                                 //Unlock
    }
}


void cleanup_music_retrieval(struct t_music_retrieval * m){
    free(m->note_array_lock);
    free(m->new_note_added);
    free(m->robot_lock);
    free(m->robot_cond);
    // cleanup the synchronization variables you created. 
    if (m->robots != NULL) free(m->robots);
}
