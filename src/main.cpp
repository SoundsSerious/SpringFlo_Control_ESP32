#include "Arduino.h"
#include <WiFi.h>
//#include "analogWrite.h"


#include "common.h"
#include "config.h"
#include "cs_timing.h"
#include "sensors.h"
#include "thermal_control.h"
#include "motion_control.h"
#include "user_input.h"
#include "wifi_status_server.h"



void print_status( void * parameters){
    Serial.begin(115200);

    while(1){


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
    Serial.println("|");

    vTaskDelay(1000);
  }

}

void setup() {
  Serial.begin(115200);

  begin_server();

  ledcSetup(therm_ledChannel, therm_freq, therm_resolution);
  ledcAttachPin(HEATER_PIN_FORMER, therm_ledChannel);
  ledcSetup(therm_ledChannel+1, therm_freq, therm_resolution);
  ledcAttachPin(HEATER_PIN_SMASHER, therm_ledChannel+1);

  pinMode(STEP_PIN,OUTPUT);
  pinMode(DIR_PIN,OUTPUT);
  pinMode(PNEUMATIC_SOLENOID_PIN,OUTPUT);
  pinMode(EXTRA_GPIO,OUTPUT);

  ledcSetup(step_ledChannel, step_freq, step_resolution);
  ledcAttachPin(STEP_PIN, step_ledChannel);


  //Start Sensor Task
  xTaskCreate(
                      read_thermocouples,          /* Task function. */
                      "Read Thermocouples",        /* String with name of task. */
                      10000,            /* Stack size in bytes. */
                      NULL,             /* Parameter passed as input of the task */
                      3,                /* Priority of the task. */
                      NULL);            /* Task handle. */

  delay(1000);


  //Start Thermal Control Tasks
  xTaskCreate(
                      smasher_control_task,          /* Task function. */
                      "Smasher Thermal Control",        /* String with name of task. */
                      10000,            /* Stack size in bytes. */
                      NULL,             /* Parameter passed as input of the task */
                      4,                /* Priority of the task. */
                      NULL);            /* Task handle. */

  xTaskCreate(
                    former_control_task,          /* Task function. */
                    "Former Thermal Control",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    4,                /* Priority of the task. */
                    NULL);            /* Task handle. */


  xTaskCreate(     check_user_input,          /* Task function. */
                    "Check user input",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    5,                /* Priority of the task. */
                    NULL);            /* Task handle. */

  xTaskCreate(     print_status,          /* Task function. */
                    "Serial Status ",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    8,                /* Priority of the task. */
                    NULL);            /* Task handle. */

}


void loop() {

  //Check Thermals
  if ( (temperature_former > PROCESS_TEMP-0.5) ){
    thermally_ready = true; }
  else{ thermally_ready = false; }

  switch(current_mode){
    case  FEED_BACK:
      open_piston();
      if ( action_button -> holding() ){ feed_back_continuous(); }
      else if (action_button -> doubleClicked()){ feed_back_one_unit(); }
      else{ feed_stop(); }
      break;
    case FEED_FORWARD:
      open_piston();
      if ( action_button -> holding() ){ feed_forward_continuous();}
      else if ( action_button -> doubleClicked() ){ feed_forward_one_unit();}
      else{ feed_stop(); }
      break;
    case IDLE:
      open_piston();
      if (action_button -> holding()){ cycle_once(); }
      break;
    case CONTINUOUS:
      if(continous_active){
        if (thermally_ready){
          //Feed Forward
          cycle_once();
        }{
          //Close Piston//
          close_piston();
          delay(100);
        }
      }
      else{
        Serial.print(".");
        delay(10);
      }
      break;
  }


}


//Start Motion Control Tasks
// xTaskCreate(
//                     piston_control,          /* Task function. */
//                     "Piston Control",        /* String with name of task. */
//                     10000,            /* Stack size in bytes. */
//                     NULL,             /* Parameter passed as input of the task */
//                   2,                /* Priority of the task. */
//                     NULL);            /* Task handle. */
//
// xTaskCreate(
//                   feed_control,          /* Task function. */
//                   "Feed Control",        /* String with name of task. */
//                   10000,            /* Stack size in bytes. */
//                   NULL,             /* Parameter passed as input of the task */
//                   2,                /* Priority of the task. */
//                   NULL);            /* Task handle. */

// xTaskCreate(     wifi_status_task,          /* Task function. */
//                   "Wifi Status Server",        /* String with name of task. */
//                   10000,            /* Stack size in bytes. */
//                   NULL,             /* Parameter passed as input of the task */
//                   6,                /* Priority of the task. */
//                   NULL);            /* Task handle. */
