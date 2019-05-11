#include "robot.h"
#include "musicretrieval.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <pthread.h>

//Write create robot
robot * create_robot(struct t_music_retrieval*music){
    robot * r = malloc(sizeof(robot));
    r->music_retrieval = music;
    // assign function pointers to defaults
    r->run = &robot_run;
    //robotAssignednum already initialized in main
    return robot;

}

void robot_run (struct t_robot * r){
    while (r->music->r->music->note_to_play_idx <= r->music->num_notes) {                   //Method does not run if current time exceeds max time     
        //pthread_mutex_lock(r->music_retrieval->robot_lock);
        //while (r->music_retrieval->added != 1) {  
        //    pthread_cond_wait(r->music_retrieval->new_note_added, r->music_retrieval->robot_lock);
        //} 
        //pthread_mutex_unlock(r->music_retrieval->robot_lock);
        //if (note_to_play == 0)  //PWM compare for 0 (depending on robot_assigned_num);
        //if (note_to_play == 1)  //PWM compare for 1;
        //if (note_to_play == 2)  //PWM compare for 2;  
        //if (note_to_play == 3)  //PWM compare for 3; 
        //if (note_to_play == 4)  //PWM compare for 4; 
        //if (note_to_play == 5)  //PWM compare for 5; 
        //if (note_to_play == 6)  //PWM compare for 6; 
        ///if (note_to_play == 7)  //PWM compare for 7;  
        if (xSemaphoreTake(r->music->new_note_added, (TickType_t) 10000) == pdTrue) {
            if (r->music->note_to_play == 0) {
                if r->robot_assigned_num == 1:
                    PWM_1_SetCompare0(20000);
                if r->robot_assigned_num == 2:
                    PWM_5_SetCompare0(20000);
                //CyDelay(500);
            } else if (r->music->note_to_play == 1) {
                if r->robot_assigned_num == 1:
                    PWM_1_SetCompare0(1500);
                if r->robot_assigned_num == 2:
                    PWM_5_SetCompare0(1500);
                //CyDelay(500);
            } else if (r->music->note_to_play == 2) {
                if r->robot_assigned_num == 1:
                    PWM_2_SetCompare0(20000);
                if r->robot_assigned_num == 2:
                    PWM_6_SetCompare0(20000);
                //CyDelay(500);
            } else if (r->music->note_to_play == 3) {
                if r->robot_assigned_num == 1:
                    PWM_2_SetCompare0(1500);
                if r->robot_assigned_num == 2:
                    PWM_6_SetCompare0(1500);
                //CyDelay(500);
            } else if (r->music->note_to_play == 4) {
                if r->robot_assigned_num == 1:
                    PWM_3_SetCompare0(20000);
                if r->robot_assigned_num == 2:
                    PWM_7_SetCompare0(20000);
                //CyDelay(500);
            } else if (r->music->note_to_play == 5) {
                if r->robot_assigned_num == 1:
                    PWM_3_SetCompare0(1500);
                if r->robot_assigned_num == 2:
                    PWM_7_SetCompare0(1500);
                //CyDelay(500);
            } else if (r->music->note_to_play == 6) {
                if r->robot_assigned_num == 1:
                    PWM_4_SetCompare0(20000);
                if r->robot_assigned_num == 2:
                    PWM_8_SetCompare0(0);
                //CyDelay(500);
            } else if (r->music->note_to_play == 7) {
                if r->robot_assigned_num == 1:
                    PWM_4_SetCompare0(1500);
                if r->robot_assigned_num == 2:
                    PWM_8_SetCompare0(1500);
                //CyDelay(500);
            }                                                  
            if (xSemaphoreTake(r->music_retrieval->robot_lock, ( TickType_t ) 1000)) {                                                  //Implement individual vehicle thread locks
                r->music_retrieval->robots_updated += 1;
                if (r->music_retrieval->robots_updated == 2) {
                    xSemaphoreGive(r->music->new_note_added);
                    r->music_retrieval->state = 0;
                } 
                xSemaphoreGive(r->music_retrieval->robot_lock);
            }
        }
        //pthread_mutex_lock(r->music_retrieval->robot_lock);
        //pthread_cond_broadcast(r->music_retrieval->robot_cond);             //wakeup musicretrieval
        //if (v->simulator->state!=1) {
        //    pthread_cond_wait(r->music_retrieval->new_note_added, r->music_retrieval->robot_lock);
        //    }
        //r->music_retrieval->state = 0;   
        //pthread_mutex_unlock(r->music_retrieval->robot_lock);                                                //Unlock
        //Increment vehicles_updated => Shared CR
        
    }




}
