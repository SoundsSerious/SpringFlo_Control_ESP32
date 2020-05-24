#include "Adafruit_MAX31855.h"
#include "SimpleKalmanFilter.h"

#include "config.h"
#include "cs_timing.h"
#include "common.h"

//Sensor Qualith Booleans
extern bool former_last_sensor_status;
extern bool former_sensor_is_nominal;


//Former Control
//extern float temperature_smasher, temp_raw_smasher;
extern float temperature_former, temp_raw_former;
extern float temperature_internal_s;
extern float temperature_internal_f;

//extern Adafruit_MAX31855 thermocouple_smasher;
extern Adafruit_MAX31855 thermocouple_former;

//extern SimpleKalmanFilter smasher_temp_filter;
extern SimpleKalmanFilter former_temp_filter;

extern unsigned long fail_time_allowed;// = 5 * one_second;//seconds
extern unsigned long fail_time_begin;// = 0.0 //seconds
extern bool heat_on; // = false

extern String failure_message;

void read_smasher_temp();
void read_former_temp();
void read_thermocouples(void * parameter);
