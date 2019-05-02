#ifndef __ROBOT_H__
#define __ROBOT_H__
#include <pthread.h>
#include "musicretrieval.h"
struct t_music_retrieval; // forward declaration
typedef struct t_robot {
    int robot_assigned_num;
    struct t_music_retrieval * music_retrieval;
    // vehicle function calls
    void (*run) (struct t_robot * r);                             // runs the vehicle update loop
} robot;



// create robot
robot * create_robot(struct t_music_retrieval*music);
// standard robot methods
void robot_run (struct t_robot * r);
#endif
