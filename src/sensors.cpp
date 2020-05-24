#include "sensors.h"


bool former_last_sensor_status = false;
bool former_sensor_is_nominal = false;

float temperature_smasher, temp_raw_smasher;
float temperature_former, temp_raw_former;
float temperature_internal_s;
float temperature_internal_f;

Adafruit_MAX31855 thermocouple_smasher = Adafruit_MAX31855(MAXCLK,MAXCS_SMASHER,MAXDO);
Adafruit_MAX31855 thermocouple_former = Adafruit_MAX31855(MAXCLK,MAXCS_FORMER,MAXDO);

SimpleKalmanFilter smasher_temp_filter = SimpleKalmanFilter(1,1,0.25);
SimpleKalmanFilter former_temp_filter = SimpleKalmanFilter(1,1,0.25);

unsigned long fail_time_allowed = 5 * one_second;//seconds
unsigned long fail_time_begin;// = 0.0 //seconds
bool heat_on; // = false

String failure_message = "";

// void read_smasher_temp(){
//   temp_raw_smasher = (float) thermocouple_smasher.readCelsius();
//   temperature_internal_s = (float) thermocouple_smasher.readInternal();
//
//   if (isnan(temp_raw_smasher)) {
//     smasher_sensor_is_nominal = false;
//     //Serial.print("Something wrong with thermocouple: ");
//     //Serial.println(thermocouple_former.readError());
//   }
//   else if(temperature_internal_f==0 && temp_raw_former == 0){
//     smasher_sensor_is_nominal = false;
//     //Serial.print("Something wrong with thermocouple: ");
//     //Serial.println(thermocouple_former.readError());
//   }
//   else{
//     smasher_sensor_is_nominal = true;
//     temperature_smasher = smasher_temp_filter.updateEstimate(temp_raw_smasher);
//   }
// }

void read_former_temp(){
  temp_raw_former = (float) thermocouple_former.readCelsius();
  temperature_internal_f = (float) thermocouple_former.readInternal();


  if (isnan(temp_raw_former)) {
    former_sensor_is_nominal = false;
    int error_message = thermocouple_former.readError();
    switch(error_message){
      case 1:
        failure_message = "ThermoCouple Open Circuit";
        break;
      case 2:
        failure_message = "ThermoCouple Short To Ground";
        break;
      case 4:
        failure_message = "ThermoCouple Short To VCC";
        break;
      default:
        failure_message = "Got Unexpected Error: "+String(error_message);
    }

  }
  else if(temperature_internal_f<=0 || temp_raw_former <= 0){
    former_sensor_is_nominal = false;
    failure_message = "ThermoCouple Reads Zero";
  }
  else{
    former_sensor_is_nominal = true;
    failure_message = "";
    temperature_former = former_temp_filter.updateEstimate(temp_raw_former);
  }

  //Failure Tolerance Loop
  if (former_sensor_is_nominal){
    heat_on = true;
    fail_time_begin = micros(); //Keep updating
  }
  else{
    if ( micros() - fail_time_begin < fail_time_allowed){
      heat_on = true;
    }
    else{
      heat_on = false;
    }
  }

}


void read_thermocouples(void * parameter){

  const TickType_t xDelay = sensor_interval_ms / portTICK_PERIOD_MS;

  thermocouple_former.begin();
  vTaskDelay(xDelay);
  //thermocouple_smasher.begin();
  //vTaskDelay(xDelay/2.0);

  for(;;){ // Always Run
    read_former_temp();
    vTaskDelay(xDelay);
    //read_smasher_temp();
    //vTaskDelay(xDelay/2.0);
  }
  vTaskDelete( NULL );
}
