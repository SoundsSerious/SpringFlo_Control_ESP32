#include "Arduino.h"
#include "common.h"
#include "config.h"
#include "cs_timing.h"
#include "sensors.h"
#include "thermal_control.h"
#include "motion_control.h"
#include "user_input.h"

void print_once(){
  Serial.print("Current Mode: ");
  switch(current_mode){
      case FEED_BACK:
        Serial.print("Backwards Feed");
        break;
      case FEED_FORWARD:
        Serial.print("Forward Feed");
        break;
      case IDLE:
        Serial.print("Idle Mode");
        break;
      case CONTINUOUS:
        Serial.print("Continuous Mode");
        break;
  }

  Serial.print(" | ");
  Serial.print(ACTION_STATE);
  Serial.print(" ");
  Serial.print(continous_active);
  Serial.print(" ");
  Serial.print(heat_on);
  Serial.print(" ");
  Serial.print((int)stepper.steps_remaining);
  Serial.print("|");

  Serial.print(" |\t");
  Serial.print(Temperature_Goal_Former);
  Serial.print(" ");
  Serial.print(temp_raw_former);
  Serial.print(" ");
  Serial.print(temperature_former);
  Serial.print(" | ");

  Serial.print(pid_former_error);
  Serial.print(" ");
  Serial.print(pid_former_ei*100.0 /255);
  Serial.print(" ");
  Serial.print(current_pwm_former*100.0 /255);
  Serial.print(" | ");

  Serial.print(pid_smasher_error);
  Serial.print(" ");
  Serial.print(pid_smasher_ei*100.0 /255);
  Serial.print(" ");
  Serial.print(current_pwm_smasher*100.0 /255);
  Serial.print(" | ");

  Serial.print(" | ");
  Serial.print(former_sensor_is_nominal);
  Serial.print(" ");
  Serial.print(thermally_ready);
  Serial.print(" ");
  Serial.print("|");

  Serial.print(" | ");
  Serial.print(do_feeding);
  Serial.print(" ");
  Serial.print(piston_closed);
  Serial.print(" ");
  Serial.print("|");
  Serial.print(WiFi.localIP());

  Serial.print("|");
  Serial.print(  ESP.getFreeHeap());
  Serial.print("|");
  Serial.println(  failure_message );



}

void print_status( void * parameters){

    for(;;){
      print_once();
      vTaskDelay((TickType_t) 1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete( NULL );
}
