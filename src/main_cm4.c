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
    // synchronization variables here
    SemaphoreHandle_t * note_array_lock;
    SemaphoreHandle_t * robot_lock;
    SemaphoreHandle_t * robot_cond;
    SemaphoreHandle_t * new_note_added;
    int state;
    int added;                      //prevent spurious wakeup
    int num_notes;                  
    int * note_array;         //num_notes in bracket
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
    music->added = 0;
    music ->note_to_play = 8;
    music->note_array_lock = NULL;
    music->new_note_added = NULL;
    music->robot_lock = NULL;
    music->robot_cond = NULL;
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
        //new_note_added makes sure we add one note at a time
        //if (xSemaphoreTake(music->new_note_added, (TickType_t) 10000) == pdTRUE) {
        //    music->added+=1;
        // pthread_cond_broadcast(music->new_note_added);
        // wait on condition variable here
        //pthread_mutex_lock(music->note_array_lock);                                                                   //Lock condition
        //while (music -> robots_updated != music -> num_robots) {                                              //When not all vehicles are updated
        //    pthread_cond_wait(music->new_note_added, music->note_array_lock);                                                  //The simulator thread will wait until 
        //}                                                                                                   //signaled that new vehicle updated
        //pthread_mutex_unlock(music->note_array_lock);   
        //    if music->num_runs != 0 {
        //       while (uxSemaphoreGetCount(music->robot_cond) != 2) {
        //            continue;
        //        }
        //    }
        
        //reset/increment music properties
        music->robots_updated = 0;
        music->current_note_idx += 1;
        music->current_note = &music->note_array[music->current_note_idx];
        music->state += 1;
        //    music->num_runs += 1
            
        }
      
        // we're past the condition (everyone is updated) so we can now broadcast to the waiting threads
        // pthread_mutex_lock(music->note_array_lock);                                                                   //Lock condition
        // music->robots_updated = 0;
        // music->current_note_idx += 1;
        // music->current+note = music->note_array[current_note_idx];
        // music->state += 1;
        // pthread_cond_broadcast(music->new_note_added);                                                              //Wake up vehicle threads so vehicle_run loop can repeat
        // pthread_mutex_unlock(music->note_array_lock);                                                                 //Unlock
    }
}

robot * create_robot(struct t_music_retrieval*music){
    robot * r = malloc(sizeof(robot));
    r->music = music;
    // assign function pointers to defaults
    r->run = &robot_run;
    //robotAssignednum already initialized in main
    return r;

}

void robot_run (struct t_robot * r){
    while (r->music->current_note_idx < r->music->num_notes) {                   //Method does not run if current time exceeds max time     
        //pthread_mutex_lock(r->music_retrieval->robot_lock);
        //while (r->music->added != 1) { 
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY );     //Wait until notified that note added
        //     }
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
            if (r->music->note_to_play == 0 ) {
                if (r->robot_assigned_num == 1) {
                    PWM_1_SetCompare0(1800);
                }
                if (r->robot_assigned_num == 2){
                    PWM_2_SetCompare0(1800);
                //CyDelay(500);
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
            if (xSemaphoreTake(r->music->robot_lock, ( TickType_t ) 1000)) {                                                  //Implement individual vehicle thread locks
                r->music->robots_updated += 1;
                if (r->music->robots_updated == 2) {
                    xTaskNotifyGive(xMusic);
                    r->music->state = 0;
                } 
                xSemaphoreGive(r->music->robot_lock);
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







void cleanup_music_retrieval(struct t_music_retrieval * m){
    free(m->note_array_lock);
    free(m->new_note_added);
    free(m->robot_lock);
    free(m->robot_cond);
    // cleanup the synchronization variables you created. 
    if (m->robots != NULL) free(m->robots);
}
    
/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  System entrance point. This function sets up user tasks and then starts 
*  the RTOS scheduler. 
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/

int main(void)
{
    __enable_irq(); /* Enable global interrupts. */
    //Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR); 
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    music_retrieval * music = create_music_retrieval();
    int num_notes = 42;
    //int note_array = malloc(num_notes * sizeof(int));
    int pitches[42] = {72,60,79,79,72,62,81,90,60,90,64,83,62,81,65,77,66,84,83,64,66,65,67,86,77,84,86,86,69,86,67,70,71,72,69,70,72,72,91,71,91,72}; //Pitch test (in decimal)
    // create 2 robots and add them to the music_retrieval
    music->num_robots = 2;
    music->robots = malloc(music->num_robots * sizeof(robot));
    music -> note_array_lock = (SemaphoreHandle_t *) malloc(sizeof(SemaphoreHandle_t));
    music -> robot_lock = (SemaphoreHandle_t *) malloc(sizeof(SemaphoreHandle_t));
    music -> robot_cond = (SemaphoreHandle_t *) malloc(sizeof(SemaphoreHandle_t));
    music -> new_note_added = (SemaphoreHandle_t *) malloc(sizeof(SemaphoreHandle_t));
    music -> note_array = pitches;
    music -> note_array_lock = xSemaphoreCreateMutex();
    music -> robot_lock = xSemaphoreCreateMutex();
    //music -> robot_cond = xSemaphoreCreateCounting();
    music -> new_note_added = xSemaphoreCreateBinary();
    
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
    
    //creating music thread
    BaseType_t xReturned2;
    xReturned2 = xTaskCreate((void*)music->run, (signed char*) "music_thread", 1024, music, tskIDLE_PRIORITY, &xMusic);
    
    //Destroying created threads
    vTaskDelete(xMusic); 
    vTaskDelete(xRobot1);
    vTaskDelete(xRobot2);
    
    //Destroy locks used
    vSemaphoreDelete(music->note_array_lock);
    vSemaphoreDelete(music->robot_lock);
    vSemaphoreDelete(music->note_array_lock);
    vSemaphoreDelete(music->new_note_added);
   
    //cleanup
    free(music);
    free(robot1);
    free(robot2);
    
    /* Start the PWM hardware block */
    //PWM_Red_Start();
    
    /* Create the queues. See the respective data-types for details of queue
       contents */
    //rgbLedDataQ = xQueueCreate(RGB_LED_QUEUE_LEN,
    //                                    sizeof(uint32_t));
    /* Create the user Tasks. See the respective Task definition for more
       details of these tasks */       
   // xTaskCreate(Task_RGB, "BLE Task", TASK_RGB_STACK_SIZE,
     //           NULL, TASK_RGB_PRIORITY, NULL);
    //xTaskCreate(Task_Touch, "Touch Task", TASK_TOUCH_STACK_SIZE,
    //            NULL, TASK_TOUCH_PRIORITY, &touchTaskHandle);
    /* Initialize thread-safe debug message printing. See uart_debug.h header 
       file to enable / disable this feature */
    //Task_DebugInit();   
    //DebugPrintf("\r\n\nPSoC 6 MCU FreeRTOS Based CapSense Design\r\n");
    //DebugPrintf("\r\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n");
    //DebugPrintf("\r\n\n");
    
    /* Start the RTOS scheduler. This function should never return */
    //xTaskCreate( vTaskCode, "Task1", 200, NULL, tskIDLE_PRIORITY, &xTask1 );
    //xTaskCreate( vTaskCode, "Task2", 200, NULL, tskIDLE_PRIORITY, &xTask2 );
    //vTaskStartScheduler();
     
    /* Should never get here! */ 
    //DebugPrintf("Error!   : RTOS - scheduler crashed \r\n");
    
    /* Halt the CPU if scheduler exits */
    //CY_ASSERT(0);
    //for(;;)
    //{
        /* Place your application code here. */
    //}
}

/* [] END OF FILE */
