#include "Adafruit_MAX31855.h"
#include "SimpleKalmanFilter.h"

#include "config.h"
#include "cs_timing.h"


//Sensor Qualith Booleans
extern bool smasher_sensor_is_nominal;
extern bool former_sensor_is_nominal;


//Former Control
extern float temperature_smasher, temp_raw_smasher;
extern float temperature_former, temp_raw_former;
extern float temperature_internal_s;
extern float temperature_internal_f;

extern Adafruit_MAX31855 thermocouple_smasher;
extern Adafruit_MAX31855 thermocouple_former;

extern SimpleKalmanFilter smasher_temp_filter;
extern SimpleKalmanFilter former_temp_filter;

void read_smasher_temp();
void read_former_temp();
void read_thermocouples(void * parameter);
