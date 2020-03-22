

#ifndef THERMAL_CONTROL_H
#define THERMAL_CONTROL_H

#include "Arduino.h"
#include "analogWrite.h"
#include "sensors.h"
#include "config.h"
#include "cs_timing.h"


const int freq = 5000;
const int resolution = 8;

void smasher_heater_pid();
void former_heater_pid();
void update_thermal_control();
void smasher_control_task(void * parameter);
void former_control_task(void * parameter);

extern bool thermally_ready;
extern float pid_Kf_p;
extern float pid_Kf_i;
extern float pid_Kf_d;

// initialize the Former
extern float Temp_Goal;
extern float Temperature_Goal_Former;


//Former Control
extern float pid_former_error, pid_former_ei, pid_former_ed;
extern float current_pwm_former;


//initialize the Smasher
extern float Temperature_Goal_Smasher;

//Former Control
extern float pid_smasher_error, pid_smasher_ei, pid_smasher_ed;
extern float current_pwm_smasher;


#endif
