#include <Arduino.h>

#ifndef COMMON_H
#define COMMON_H


extern int CLOSE_TIME; //Should be between 0.3-3 seconds
extern int PROCESS_TEMP;
extern int FEED_TIME;
extern int EJECT_TIME;
extern int PISTON_TIME;
extern int DELAY_TIME;

enum ACTION_MODES {FEED_BACK,FEED_FORWARD,IDLE,CONTINUOUS};
extern enum ACTION_MODES current_mode;

extern String current_mode_name( ACTION_MODES mode);

extern bool continous_active;

#endif




//#include "Arduino.h"
// #include "analogWrite.h"
// #include "SPI.h"
// #include "Adafruit_MAX31855.h"
// #include "SimpleKalmanFilter.h"
// #include "config.h"
// #include "cs_timing.h"
//
// // Default connection is using software SPI, but comment and uncomment one of
// // the two examples below to switch between software SPI and hardware SPI:
//
// // Example creating a thermocouple instance with software SPI on any three
// // digital IO pins.
