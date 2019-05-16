/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.00
*
* Description: The project includes CapSense configuration implementing two 
* buttons and a five-element slider, FreeRTOS configuration on Cortex-M4 and
* FreeRTOS task implementation for CapSense and LED interface. In addition, 
* the project defines and implements the CapSense exit/entry callbacks through
* the “cyapicallbacks.h” file. The CapSense task initializes the CapSense block,
* scans the widgets periodically, processes the widgets after each scan and 
* notifies the LED task to display the output. The LED task controls the RGB LED.
* The slider value controls the intensity and the buttons control ON/OFF of the
* LEDs. The CapSense entry/exit callbacks provide the CapSense task resume/wakeup
* functionality on CapSense scan completion. 
*
* Related Document: 
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit
*                     
*
*******************************************************************************
* Copyright (2018), Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress 
* reserves the right to make changes to the Software without notice. Cypress 
* does not assume any liability arising out of the application or use of the 
* Software or any product or circuit described in the Software. Cypress does 
* not authorize its products for use in any products where a malfunction or 
* failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such 
* system or application assumes all risk of such use and in doing so agrees to 
* indemnify Cypress against all liability.
*******************************************************************************/
/******************************************************************************
* This code example demonstrates the capabilities of the PSoC 6 BLE to 
* communicate bi-directionally with a BLE Central device over custom services, 
* while performing CapSense touch sensing and updating GUI elements such as an 
* RGB LED and an E-INK display. The CapSense custom service allows notifications  
* to be sent to the central device when notifications are enabled. On the other  
* hand, the RGB LED custom service allows read and write of attributes under the 
* RGB characteristics.
*
* This project utilizes CapSense component to read touch information from a
* slider two buttons button and then report this to central device over BLE.  
* On the other hand, the control values sent to PSoC 6 BLE is converted to  
* respective color and intensity values and displayed using the on-board  
* RGB LED. The BLE central device can be CySmart mobile app or CySmart BLE 
* Host Emulation PC tool. 
*
* This code example uses FreeRTOS. For documentation and API references of 
* FreeRTOS, visit : https://www.freertos.org 
*
*******************************************************************************/
/* Header file includes */
/* Header file includes */ 
#include "project.h"
#include "FreeRTOS.h"  
//#include "task_touch.h"
//#include "task_rgb.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "uart_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "semphr.h"
#include <stdbool.h>

//#include "musicretrieval.h"
//#include "robot.h"

TaskHandle_t xMusic = NULL;

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

struct t_music_retrieval; // forward declaration
typedef struct t_robot {
    int robot_assigned_num;
    struct t_music_retrieval * music;
    // vehicle function calls
    void (*run) (struct t_robot * r);                             // runs the vehicle update loop
} robot;

// create robot
robot * create_robot(struct t_music_retrieval*music);
// standard robot methods
void robot_run (struct t_robot * r);

void music_retrieval_convert(struct t_music_retrieval * music, int musical_note){
// music->note_to_play = x
    if (musical_note == 60) {
        music->note_to_play = 0;
    } else if (musical_note == 62 || musical_note == 74 || musical_note == 86) {
        music->note_to_play = 1;
    } else if (musical_note == 64 || musical_note == 76 || musical_note == 88) {
        music->note_to_play = 2;
    } else if (musical_note == 65 || musical_note == 77 || musical_note == 89) {
        music->note_to_play = 3;
    } else if (musical_note == 67 || musical_note == 79 || musical_note == 91) {
        music->note_to_play = 4;
    } else if (musical_note == 69 || musical_note == 81 || musical_note == 93) {
        music->note_to_play = 5;
    } else if (musical_note == 71 || musical_note == 83 || musical_note == 95) {
        music->note_to_play = 6;
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
    //music->num_runs = 0;
    // initialize your synchronization variables here!
    music->note_array_lock = NULL;
    music->new_note_added = NULL;
    music->robot_lock = NULL;
    music->robot_cond = NULL;
    return music;
}

void run(struct t_music_retrieval * music){
    BaseType_t xReturned2;
    TaskHandle_t xMusic = NULL;
    xReturned2 = xTaskCreate((void*)music->run, (signed char*) "music_thread", 1024, music, 1, &xMusic);
    BaseType_t xReturned;
    TaskHandle_t xRobot1 = NULL;
    xReturned = xTaskCreate((void *)music->robots[1].run, (signed char*) "robot_thread", 1024, &(music -> robots[1]), 1, &xRobot1);
    BaseType_t xReturned1;
    TaskHandle_t xRobot2 = NULL;
    xReturned1 = xTaskCreate((void *)music->robots[1].run, (signed char*) "robot_thread", 1024, &(music -> robots[1]), 1, &xRobot2);
    while (music->current_note_idx <= music->num_notes) {
        music->music_retrieval_convert(music, *music->current_note);
        //new_note_added makes sure we add one note at a time
        if (xSemaphoreTake(music->new_note_added, (TickType_t) 10000) == pdTRUE) {
            music->added+=1;
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
            music->robots_updated = 0;
            music->current_note_idx += 1;
            music->current_note = &music->note_array[music->current_note_idx];
            music->state += 1;
        //    music->num_runs += 1
            xSemaphoreGive(music->new_note_added);
        }
        xTaskNotifyGive(xRobot1);
        xTaskNotifyGive(xRobot2);
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
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
    while (r->music->current_note_idx <= r->music->num_notes) {                   //Method does not run if current time exceeds max time     
        //pthread_mutex_lock(r->music_retrieval->robot_lock);
        while (r->music->added != 1) { 
                 ulTaskNotifyTake( pdTRUE, portMAX_DELAY );     //Wait until notified that note added
             }
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


/* Stack sizes of user tasks in this project */
#define TASK_RGB_STACK_SIZE      (configMINIMAL_STACK_SIZE)
#define TASK_TOUCH_STACK_SIZE    (configMINIMAL_STACK_SIZE)
    
/* Priorities of user tasks in this project - spaced at intervals of 5 for 
the ease of further modification and addition of new tasks. 
Larger number indicates higher priority. */
#define TASK_RGB_PRIORITY       1
#define TASK_TOUCH_PRIORITY     5
    
 /* Maximum number of messages that can be queued */
#define RGB_LED_QUEUE_LEN    (1) 
    
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
//static void prvTask1( void *pvParameters );
//static void prvTask2( void *pvParameters );
void vTaskCode( void * pvParameters )
{
    // The parameter value is expected to be 1 as 1 is passed in the
    //pvParameters value in the call to xTaskCreate() below. 
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
    int x = 0;
    for( ;; )
    {
        /* Task code goes here. */
        x = x+1;
    }
}

/* Handles for the tasks create by main(). */
static TaskHandle_t xTask1 = NULL, xTask2 = NULL;

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
    //music -> note_array_lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    music -> note_array_lock = (SemaphoreHandle_t *) malloc(sizeof(SemaphoreHandle_t));
    //music -> robot_lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    music -> robot_lock = (SemaphoreHandle_t *) malloc(sizeof(SemaphoreHandle_t));
    //music -> robot_cond = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    music -> robot_cond = (SemaphoreHandle_t *) malloc(sizeof(SemaphoreHandle_t));
    //music -> new_note_added = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    music -> new_note_added = (SemaphoreHandle_t *) malloc(sizeof(SemaphoreHandle_t));
    music -> note_array = pitches;
    music -> note_array_lock = xSemaphoreCreateMutex();
    //pthread_mutex_init(music -> robot_lock, NULL);
    music -> robot_lock = xSemaphoreCreateMutex();
    //pthread_cond_init(music -> robot_cond, NULL);
    //music -> robot_cond = xSemaphoreCreateCounting();
    //pthread_cond_init(music -> new_note_added, NULL);
    music -> new_note_added = xSemaphoreCreateBinary();
    //pthread_t thread_ids [music->num_robots];
    TaskHandle_t thread_ids [music->num_robots];
    PWM_1_Start();
    PWM_2_Start();
    PWM_3_Start();
    PWM_4_Start();
    //PWMs for Robot 2's servos 1-4
    robot * robot0 = create_robot(music);
    robot0->robot_assigned_num = 0;
        // creates a copy so we can free the original vehicle after
    music->robots[0] = *robot0;
    free(robot0);
    //BaseType_t xReturned;
    //TaskHandle_t xRobot1 = NULL;
    //xReturned = xTaskCreate((void *)music->robots[0].run, (signed char*) "robot_thread", 1024, &(music -> robots[0]), 1, &xRobot1);
    robot * robot1 = create_robot(music);
    robot1->robot_assigned_num = 1;
        // creates a copy so we can free the original vehicle after
    music->robots[1] = *robot1;
    free(robot1);
    
    //BaseType_t xReturned1;
    //TaskHandle_t xRobot2 = NULL;
    //xReturned1 = xTaskCreate((void *)music->robots[1].run, (signed char*) "robot_thread", 1024, &(music -> robots[1]), 1, &xRobot2);
    BaseType_t xReturned2;
    //TaskHandle_t xMusic = NULL;
    xReturned2 = xTaskCreate((void*)music->run, (signed char*) "music_thread", 1024, music, 1, &xMusic);
    //vTaskDelete(xMusic); // join the threads we created
    //vTaskDelete(xRobot1);
    //vTaskDelete(xRobot2);
    //pthread_mutex_destroy(music -> note_array_lock);   //Destroy mutex locks used
    vSemaphoreDelete(music->note_array_lock);
    //pthread_mutex_destroy(music -> robot_lock);   //Destroy mutex locks used
    vSemaphoreDelete(music->robot_lock);
    //pthread_cond_destroy(music -> robot_cond);
    vSemaphoreDelete(music->note_array_lock);
    //pthread_cond_destroy(music -> new_note_added);
    vSemaphoreDelete(music->new_note_added);
    // cleanup - will free memory used by vehicles and simulator.
    //cleanup_simulator(music);
    free(music);
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
/*******************************************************************************
* Function Name: void vApplicationIdleHook(void)
********************************************************************************
*
* Summary:
*  This function is called when the RTOS in idle mode
*    
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void vApplicationIdleHook(void)
{
    /* Enter sleep-mode */
    Cy_SysPm_Sleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
}

/*******************************************************************************
* Function Name: void vApplicationStackOverflowHook(TaskHandle_t *pxTask, 
                                                    signed char *pcTaskName)
********************************************************************************
*
* Summary:
*  This function is called when a stack overflow has been detected by the RTOS
*    
* Parameters:
*  TaskHandle_t  : Handle to the task
*  signed char   : Name of the task
*
* Return:
*  None
*
*******************************************************************************/
void vApplicationStackOverflowHook(TaskHandle_t *pxTask, 
                                   signed char *pcTaskName)
{
    /* Remove warning for unused parameters */
    (void)pxTask;
    (void)pcTaskName;
    
    /* Print the error message with task name if debug is enabled in 
       uart_debug.h file */
    DebugPrintf("Error!   : RTOS - stack overflow in %s \r\n", pcTaskName);
    
    /* Halt the CPU */
    CY_ASSERT(0);
}

/*******************************************************************************
* Function Name: void vApplicationMallocFailedHook(void)
********************************************************************************
*
* Summary:
*  This function is called when a memory allocation operation by the RTOS
*  has failed
*    
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void vApplicationMallocFailedHook(void)
{
    /* Print the error message if debug is enabled in uart_debug.h file */
    DebugPrintf("Error!   : RTOS - Memory allocation failed \r\n");
    
    /* Halt the CPU */
    CY_ASSERT(0);
}
/* [] END OF FILE */
