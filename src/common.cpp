#include "common.h"


int CLOSE_TIME = 1000; //ms//Should be between 0.3-3 seconds
int PROCESS_TEMP = 130; //C
int FEED_TIME = 100; //ms
int EJECT_TIME = 250;
int PISTON_TIME = 100;
int DELAY_TIME = 50;

ACTION_MODES current_mode = FEED_BACK;

bool continous_active = false;

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
