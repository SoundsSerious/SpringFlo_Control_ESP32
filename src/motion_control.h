#ifndef MOTION_CONTROL_H
#define MOTION_CONTROL_H

#include "Arduino.h"
#include "config.h"
#include "cs_timing.h"
#include "a4988.h"

#define MOTOR_STEPS 200

extern bool do_feeding;
extern bool piston_closed;

extern float feed_distance; //mm
extern float feed_diameter;
extern float pi;
extern float feed_circumfrence;

extern float steps_per_rotation;

extern float distance_per_step;
extern int steps_per_feed;

extern float feed_degrees_rotation;
extern long steps_to_go;
extern bool step_state;

//Keep A Runing Count Of How Much To Add or subtract,
//Add to this number to increase distance, subtract when they are made..?

extern A4988 stepper;



void pison_control(void * parameters );

void feed_control(void * parameters );

#endif