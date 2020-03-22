#include "Arduino.h"
#include <WiFi.h>
#include "analogWrite.h"


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
    Serial.println(current_mode);

    Serial.print("MODE: ");
    Serial.println((int)MODE_STATE);

    Serial.print("FEED: ");
    Serial.println((int)FEED_STATE);

    Serial.print("ACTION: ");
    Serial.print(action_button->doubleClicked());
    Serial.print(action_button->clicked());
    Serial.println(action_button->holding());

    Serial.print(Temperature_Goal_Former);
    Serial.print(" ");
    Serial.print(temp_raw_former);
    Serial.print(" ");
    Serial.print(temperature_former);
    Serial.print(" | ");

    Serial.print(pid_former_error*100.0 /255);
    Serial.print(" ");
    Serial.print(pid_former_ei*100.0 /255);
    Serial.print(" ");
    Serial.print(current_pwm_former*100.0 /255);
    Serial.print(" | ");

    Serial.print(pid_smasher_error*100.0 /255);
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

  analogWriteResolution(8);
  analogWriteFrequency(1000);

  pinMode(STEP_PIN,OUTPUT);
  pinMode(DIR_PIN,OUTPUT);
  pinMode(PNEUMATIC_SOLENOID_PIN,OUTPUT);
  pinMode(EXTRA_GPIO,OUTPUT);


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

  xTaskCreate(     wifi_status_task,          /* Task function. */
                    "Wifi Status Server",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    6,                /* Priority of the task. */
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

void feed_forward_one_unit(){
  Serial.println("Feed Forward");
  digitalWrite(DIR_PIN,HIGH);
  steps_to_go = steps_per_feed;
  if (steps_to_go > 0){
      while(steps_to_go>0){
        step_state = !step_state;
        digitalWrite(STEP_PIN,step_state);
        delay(1);
        steps_to_go = steps_to_go - 1;
      }
  }
}

void feed_forward_continuous(){
  Serial.print(">");
  digitalWrite(DIR_PIN,HIGH);
  steps_to_go = 10;
  if (steps_to_go > 0){
      while(steps_to_go>0){
        step_state = !step_state;
        digitalWrite(STEP_PIN,step_state);
        delay(1);
        steps_to_go = steps_to_go - 1;
      }
  }
}

void feed_back_continuous(){
  Serial.print("<");
  digitalWrite(DIR_PIN,LOW);
  steps_to_go = 10;
  if (steps_to_go > 0){
      while(steps_to_go>0){
        step_state = !step_state;
        digitalWrite(STEP_PIN,step_state);
        delay(1);
        steps_to_go = steps_to_go - 1;
      }
  }
}

void feed_back_one_unit(){
  Serial.println("Feed Back");
  digitalWrite(DIR_PIN,LOW);
  steps_to_go = steps_per_feed;
  if (steps_to_go > 0){
      while(steps_to_go>0){
        step_state = !step_state;
        digitalWrite(STEP_PIN,step_state);
        delay(1);
        steps_to_go = steps_to_go - 1;
      }
  }
}

void close_piston(){
  Serial.println("Close");
  digitalWrite(PNEUMATIC_SOLENOID_PIN,HIGH);
  delay(100);
}

void open_piston(){
  Serial.println("Open");
  digitalWrite(PNEUMATIC_SOLENOID_PIN,LOW);
  delay(100);
}

void cycle_once(){
    Serial.println("Cycle Once");
    feed_forward_one_unit();
    delay(50);
    //Close
    close_piston();
    //Wait
    delay(CLOSE_TIME);
    //Open
    open_piston();
    //Eject
    delay(250);
}



void loop() {

  if ( (temperature_former > Temperature_Goal_Smasher-5.0) ){
    thermally_ready = true; }
  else{ thermally_ready = false; }

  switch(current_mode){
    case  FEED_BACK:
      if ( action_button -> holding() ){ feed_back_continuous(); }
      break;
    case FEED_FORWARD:
      if ( action_button -> holding() ){ feed_forward_continuous();}
      break;
    case IDLE:
      if (action_button -> holding()){
        cycle_once();
      }
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
