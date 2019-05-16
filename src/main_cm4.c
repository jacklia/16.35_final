/******************************************************************************
* File Name: main_cm4.c
*******************************************************************************/
/* Header file includes */
/* Header file includes */ 
#include "project.h"
#include "FreeRTOS.h"  
#include "task.h"
#include "queue.h"
#include "timers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "semphr.h"
#include <stdbool.h>

//#include "musicretrieval.h"
//#include "robot.h"

//music thread handle
TaskHandle_t xMusic = NULL;
//robot1 thread handle
TaskHandle_t xRobot1 = NULL;
//robot2 thread handle
TaskHandle_t xRobot2 = NULL;

typedef struct t_music_retrieval{
    // make sure robots_updated gets updated by one robot thread at a time
    SemaphoreHandle_t * robot_lock;
    // number of notes in song
    int num_notes;   
    // array of notes
    int * note_array;         
    // current note being played
    int * current_note;
    int current_note_idx;
    //number of robots updated in each run
    int robots_updated; // condition is robots_updated == num_robots.
    //number of robots in total
    int num_robots;
    // integer (0-7) representation of note
    int note_to_play;

    struct t_robot * robots;
    // music_retrieval run function
    void    (*run)(struct t_music_retrieval * m);
    // function to convert notes from decimal to integer between 0-7
    void    (*music_retrieval_convert)(struct t_music_retrieval * m, int musical_note);         //if change to hex do unsigned char 
} music_retrieval;

// create music 
music_retrieval * create_music_retrieval();
// standard music methods
void run(struct t_music_retrieval * m);
void cleanup_music_retrieval(struct t_music_retrieval * m);

typedef struct t_robot {
    //either robot 1 or robot 2
    int robot_assigned_num;
    struct t_music_retrieval * music;
    // robot function calls
    void (*run) (struct t_robot * r);                             
} robot;

// create robot
robot * create_robot(struct t_music_retrieval*music);
// standard robot methods
void robot_run (struct t_robot * r);

//converts music from its decimal representation to its integer representation
void music_retrieval_convert(struct t_music_retrieval * music, int musical_note){
    //Low C
    if (musical_note == 60) {
        music->note_to_play = 0;
    //D
    } else if (musical_note == 62 || musical_note == 74 || musical_note == 86) {
        music->note_to_play = 1;
    //E
    } else if (musical_note == 64 || musical_note == 76 || musical_note == 88) {
        music->note_to_play = 2;
    //F
    } else if (musical_note == 65 || musical_note == 77 || musical_note == 89) {
        music->note_to_play = 3;
    //G
    } else if (musical_note == 67 || musical_note == 79 || musical_note == 91) {
        music->note_to_play = 4;
    //A
    } else if (musical_note == 69 || musical_note == 81 || musical_note == 93) {
        music->note_to_play = 5;
    //B
    } else if (musical_note == 71 || musical_note == 83 || musical_note == 95) {
        music->note_to_play = 6;
    //High C
    } else if (musical_note == 72 || musical_note == 84) {
        music->note_to_play = 7;
    }
}

music_retrieval * create_music_retrieval(){
    // initialization
    music_retrieval * music = malloc(sizeof(music_retrieval));
    music->run = &run;
    music->music_retrieval_convert = &music_retrieval_convert;
    music->num_robots = 0;
    music->num_notes = 0;
    music->current_note_idx = 0;
    music->robots_updated = 0;
    music->note_array = malloc(music->num_notes * sizeof(int));
    music->current_note = &music->note_array[0];
    music->robots = NULL;
    music ->note_to_play = 8;
    music->robot_lock = NULL;
    return music;
}

void run(struct t_music_retrieval * music){
    //creating thread for robot 1
    BaseType_t xReturned;
    xReturned = xTaskCreate((void *)music->robots[0].run, (signed char*) "robot1_thread", 1024, &(music -> robots[0]), tskIDLE_PRIORITY, &xRobot1);
    
    //creating thread for robot 2
    BaseType_t xReturned1;
    xReturned1 = xTaskCreate((void *)music->robots[1].run, (signed char*) "robot2_thread", 1024, &(music -> robots[1]), tskIDLE_PRIORITY, &xRobot2);
    
    //while there are notes left to play
    while (music->current_note_idx <= music->num_notes) {
        //setting music->note_to_play to current note
        music->music_retrieval_convert(music, *music->current_note);
        
        //signal to robots to run their methods
        xTaskNotifyGive(xRobot1);
        xTaskNotifyGive(xRobot2);
        
        //block to wait for robots to signal that they're done 
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
        
        //reset/increment music properties
        music->robots_updated = 0;
        music->current_note_idx += 1;
        music->current_note = &music->note_array[music->current_note_idx];           
        }
}

robot * create_robot(struct t_music_retrieval*music){
    robot * r = malloc(sizeof(robot));
    r->music = music;
    // assign function pointers to defaults
    r->run = &robot_run;
    //robot_assigned_num initialized in main
    return r;

}

void robot_run (struct t_robot * r){
    // while there are notes left to play
    while (r->music->current_note_idx < r->music->num_notes) {                     
        //Wait until notified that note added
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY );   
        //assign notes to movements
        if (r->music->note_to_play == 0 ) {
            if (r->robot_assigned_num == 1) {
                PWM_1_SetCompare0(1800);
            }
            if (r->robot_assigned_num == 2){
                PWM_2_SetCompare0(1800);
            }
            CyDelay(500);
        } else if (r->music->note_to_play == 1) {
            if (r->robot_assigned_num == 1){
                PWM_1_SetCompare0(1200);
            }
            if (r->robot_assigned_num == 2){
                PWM_2_SetCompare0(1200);
            }
            CyDelay(500);
        } else if (r->music->note_to_play == 2) {
            if (r->robot_assigned_num == 1){
                PWM_3_SetCompare0(1200);
            }
            if (r->robot_assigned_num == 2){
                PWM_4_SetCompare0(1200);
            }
            CyDelay(500);
        } else if (r->music->note_to_play == 3) {
            if (r->robot_assigned_num == 1) {
                PWM_3_SetCompare0(1800);
            }
            if (r->robot_assigned_num == 2) {
                PWM_4_SetCompare0(1800);
            }
            CyDelay(500);
        //moves repeat since we only have 4 servos working
        } else if (r->music->note_to_play == 4) {
            if (r->robot_assigned_num == 1) {
                PWM_1_SetCompare0(1800);
            }
            if (r->robot_assigned_num == 2){
                PWM_2_SetCompare0(1800);
            }
            CyDelay(500);
        } else if (r->music->note_to_play == 5) {
            if (r->robot_assigned_num == 1){
                PWM_1_SetCompare0(1200);
            }
            if (r->robot_assigned_num == 2){
                PWM_2_SetCompare0(1200);
            }
            CyDelay(500);
        } else if (r->music->note_to_play == 6) {
            if (r->robot_assigned_num == 1){
                PWM_3_SetCompare0(1200);
            }
            if (r->robot_assigned_num == 2){
                PWM_4_SetCompare0(1200);
            }
            CyDelay(500);
        } else if (r->music->note_to_play == 7) {
            if (r->robot_assigned_num == 1) {
                PWM_3_SetCompare0(1800);
            }
            if (r->robot_assigned_num == 2) {
                PWM_4_SetCompare0(1800);
            }
            CyDelay(500);
        } 
        // make sure only one robot updates music->robots_updated at once
        if (xSemaphoreTake(r->music->robot_lock, ( TickType_t ) 1000)) {                                                  
            r->music->robots_updated += 1;
            // if both robots updated, notify music thread it can move to next note
            if (r->music->robots_updated == 2) {
                xTaskNotifyGive(xMusic);
                r->music->state = 0;
            } 
            xSemaphoreGive(r->music->robot_lock);
        }
    }
}

int main(void) {
    __enable_irq(); /* Enable global interrupts. */
    //Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR); 
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    music_retrieval * music = create_music_retrieval();
    music->num_notes = 42;
    // hard-coded array of notes
    int pitches[music->num_notes] = {72,60,79,79,72,62,81,90,60,90,64,83,62,81,65,77,66,84,83,64,66,65,67,86,77,84,86,86,69,86,67,70,71,72,69,70,72,72,91,71,91,72}; //Pitch test (in decimal)
    // create 2 robots and other music properties
    music->num_robots = 2;
    music->robots = malloc(music->num_robots * sizeof(robot));
    music -> robot_lock = (SemaphoreHandle_t *) malloc(sizeof(SemaphoreHandle_t));
    music -> note_array = pitches;
    music -> robot_lock = xSemaphoreCreateMutex();
    
    //Keep track of robot threads
    TaskHandle_t thread_ids [music->num_robots];
    
    //PWMs for Robot 1's servos 1, 3
    PWM_1_Start();
    PWM_2_Start();
    //PWMs for Robot 2's servos 1, 3
    PWM_3_Start();
    PWM_4_Start();
    
    //creating first robot
    robot * robot1 = create_robot(music);
    robot1->robot_assigned_num = 0;
    music->robots[0] = *robot1;
    free(robot1);
    
    //creating second robot
    robot * robot2 = create_robot(music);
    robot2->robot_assigned_num = 1;
    music->robots[1] = *robot2;
    free(robot2);
    
    //creating and running music thread
    BaseType_t xReturned2;
    xReturned2 = xTaskCreate((void*)music->run, (signed char*) "music_thread", 1024, music, tskIDLE_PRIORITY, &xMusic);
    
    //Destroying created threads
    vTaskDelete(xMusic); 
    vTaskDelete(xRobot1);
    vTaskDelete(xRobot2);
    
    //Destroy locks used
    vSemaphoreDelete(music->robot_lock);
   
    //cleanup
    free(music);
    free(music->robot_lock)
    free(robot1);
    free(robot2);
}

/* [] END OF FILE */
