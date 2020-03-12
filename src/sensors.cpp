#include "sensors.h"


bool smasher_sensor_is_nominal = false;
bool former_sensor_is_nominal = false;

float temperature_smasher, temp_raw_smasher;
float temperature_former, temp_raw_former;
float temperature_internal_s;
float temperature_internal_f;

Adafruit_MAX31855 thermocouple_smasher = Adafruit_MAX31855(MAXCLK,MAXCS_SMASHER,MAXDO);
Adafruit_MAX31855 thermocouple_former = Adafruit_MAX31855(MAXCLK,MAXCS_FORMER,MAXDO);

SimpleKalmanFilter smasher_temp_filter = SimpleKalmanFilter(1,1,0.25);
SimpleKalmanFilter former_temp_filter = SimpleKalmanFilter(1,1,0.25);

void read_smasher_temp(){
  temp_raw_smasher = (float) thermocouple_smasher.readCelsius();
  temperature_internal_s = (float) thermocouple_smasher.readInternal();

  if (isnan(temp_raw_smasher)) {
    smasher_sensor_is_nominal = false;
    Serial.print("Something wrong with thermocouple: ");
    Serial.println(thermocouple_former.readError());
  }
  else if(temperature_internal_f==0 && temp_raw_former == 0){
    smasher_sensor_is_nominal = false;
    Serial.print("Something wrong with thermocouple: ");
    Serial.println(thermocouple_former.readError());
  }
  else{
    smasher_sensor_is_nominal = true;
    temperature_smasher = smasher_temp_filter.updateEstimate(temp_raw_smasher);
  }
}

void read_former_temp(){
  temp_raw_former = (float) thermocouple_former.readCelsius();
  temperature_internal_f = (float) thermocouple_former.readInternal();

  if (isnan(temp_raw_former)) {
    former_sensor_is_nominal = false;
    Serial.print("Something wrong with thermocouple: ");
    Serial.println(thermocouple_former.readError());
  }
  else if(temperature_internal_f==0 && temp_raw_former == 0){
    former_sensor_is_nominal = false;
    Serial.print("Something wrong with thermocouple: ");
    Serial.println(thermocouple_former.readError());
  }
  else{
    former_sensor_is_nominal = true;
    temperature_former = former_temp_filter.updateEstimate(temp_raw_former);
  }
}


void read_thermocouples(void * parameter){

  const TickType_t xDelay = sensor_interval_ms / portTICK_PERIOD_MS;

  thermocouple_former.begin();
  vTaskDelay(xDelay/2.0);
  thermocouple_smasher.begin();
  vTaskDelay(xDelay/2.0);

  while(1){ // Always Run
    read_former_temp();
    vTaskDelay(xDelay/2.0);
    read_smasher_temp();
    vTaskDelay(xDelay/2.0);
  }
}
