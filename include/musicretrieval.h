#ifndef __MUSICRETRIEVAL_H__
#define __MUSICRETRIEVAL_H__
#include "robot.h"
#include <pthread.h>
typedef struct t_music_retrieval{
    // synchronization variables here
    //pthread_mutex_t * note_array_lock;
    SemaphoreHandle_t * note_array_lock;
    //pthread_mutex_t * robot_lock;
    SemaphoreHandle_t * robot_lock;
    //pthread_cond_t * robot_cond;
    SemaphoreHandle_t * robot_cond;
    //pthread_cond_t * new_note_added;
    SemaphoreHandle_t * new_note_added;
    int state;
    int added;                      //prevent spurious wakeup
    // don't change these
    int num_notes;                  
    int note_array[42];         //num_notes in bracket
    int * current_note;
    int current_note_idx;
    int robots_updated; // condition is robots_updated == num_robots.
    int num_robots;
    int note_to_play;
    int num_runs;
    struct t_robot * robots;
    // music_retrieval run function
    void    (*run)(struct t_music_retrieval * m);
    void    (*music_retrieval_convert)(struct t_music_retrieval * m, int musical_note);         //if change to hex do unsigned char 
} music_retrieval;

music_retrieval * create_music_retrieval();
void run(struct t_music_retrieval * m);
void cleanup_music_retrieval(struct t_music_retrieval * m);
#endif
