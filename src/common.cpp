#include "common.h"


int CLOSE_TIME = 750; //ms//Should be between 0.3-3 seconds
int PROCESS_TEMP = 130; //C
int FEED_TIME = 10; //ms
int EJECT_TIME = 100;
int PISTON_TIME = 50;
int DELAY_TIME = 10;

int MOTOR_RPM = 110;
int MOTOR_ACL = 1000;
int MOTOR_DCL = 5000;

float K_P = 150.0;
float K_I = 1.0;

ACTION_MODES current_mode = FEED_BACK;

bool continous_active = false;

float temp_past_minute;
bool last_thermally_active_state;

float instantnious_cycle_time = (CLOSE_TIME + PISTON_TIME * 2 + EJECT_TIME)/1000.0;
float instantnious_production_rate=0;
float past_hour_production_rate=0;
unsigned long cumulative_cycles = 0;

unsigned long one_second = 1000000; //microseconds
unsigned long one_minute = one_second*5; //microseconds

String current_mode_name( ACTION_MODES mode){
  String name;
    switch(mode){
      case FEED_BACK:
        name = "Backwards Feed (On Press)";
        break;
      case FEED_FORWARD:
        name = "Forward Feed (On Press)";
        break;
      case IDLE:
        name = "Idle (Press To Cycle Once)";
        break;
      case CONTINUOUS:
        name = "Continous Operation (Press To Start / Pause)";
        break;
    }
    return name;
};
