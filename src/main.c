#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "musicretrieval.h"
#include "robot.h"
#include <pthread.h>
#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"

int main(int argc, char *argv[])
{
    // this launches the display server (in a separate process)
   
    // create the simulator
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
    music -> note_array[42] = *pitches;
    //pthread_mutex_init( music -> note_array_lock, NULL);
    music -> note_array_lock = xSemaphoreCreateMutex();
    //pthread_mutex_init(music -> robot_lock, NULL);
    music -> robot_lock = xSemaphoreCreateMutex();
    //pthread_cond_init(music -> robot_cond, NULL);
    music -> robot_cond = xSemaphoreCreateCounting();
    //pthread_cond_init(music -> new_note_added, NULL);
    music -> new_note_added = xSemaphoreCreateBinary();
    //pthread_t thread_ids [music->num_robots];
    TaskHandle_t thread_ids [music->num_robots];
    //PWMs for Robot 1's servos 1-4
    PWM_1_Start();
    PWM_2_Start();
    PWM_3_Start();
    PWM_4_Start();
    //PWMs for Robot 2's servos 1-4
    PWM_5_Start();
    PWM_6_Start();
    PWM_7_Start();
    PWM_8_Start();
    for (int i = 0; i < music->num_robots; i++) {
        robot * robot = create_robot(music);
        robot->robot_assigned_num = i;
        // creates a copy so we can free the original vehicle after
        music->robots[i] = *robot;
        free(robot);
        // create the threads using thread_ids[i]
        //pthread_create(&thread_ids[i], NULL, (void *)music->robots[i].run, &(music -> robots[i]));
        BaseType_t xReturned;
        TaskHandle_t xHandle = NULL;
        xReturned = xTaskCreate((void *)music->robots[i].run, (signed char*) "robot_thread", 1024, &(music -> robots[i]), 1, &xHandle);
        thread_ids.append(xHandle);

    }
    music->run(music);
    //for (int i = 0; i < music -> num_robots; i++) pthread_join(thread_ids[i],NULL); // join the threads we created
    for (int i = 0; i < music -> num_robots; i++) vTaskDelete(thread_ids[i]); // join the threads we created
    //pthread_mutex_destroy(music -> note_array_lock);   //Destroy mutex locks used
    vSemaphoreDelete(music->note_array_lock);
    //pthread_mutex_destroy(music -> robot_lock);   //Destroy mutex locks used
    vSemaphoreDelete(music->robot_lock);
    //pthread_cond_destroy(music -> robot_cond);
    vSemaphoreDelete(music->note_array_lock);
    //pthread_cond_destroy(music -> new_note_added);
    vSemaphoreDelete(music->new_note_added);
    // cleanup - will free memory used by vehicles and simulator.
    cleanup_simulator(music);
    free(music);
}
