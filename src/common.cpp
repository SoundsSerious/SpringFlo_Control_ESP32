#include "common.h"


int CLOSE_TIME = 350; //ms//Should be between 0.3-3 seconds
int PROCESS_TEMP = 130; //C
int FEED_TIME = 10; //ms
int EJECT_TIME = 100;
int PISTON_TIME = 25;
int DELAY_TIME = 20;

int MOTOR_RPM = 300;
int MOTOR_ACL = 20000;
int MOTOR_DCL = 20000;

float K_P = 50.0;
float K_I = 0.05;

ACTION_MODES current_mode = FEED_BACK;

int MAX_TEMP = 150;

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

void feedTheDog(){
  // feed dog 0
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE; // write enable
  TIMERG0.wdt_feed=1;                       // feed dog
  TIMERG0.wdt_wprotect=0;                   // write protect
  // feed dog 1
  TIMERG1.wdt_wprotect=TIMG_WDT_WKEY_VALUE; // write enable
  TIMERG1.wdt_feed=1;                       // feed dog
  TIMERG1.wdt_wprotect=0;                   // write protect
}
