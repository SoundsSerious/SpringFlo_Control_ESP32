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

unsigned long last_time;
unsigned long this_time;
float dt;
unsigned long last_count;


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
      Serial.println("|");

      vTaskDelay((TickType_t) 1000 / portTICK_PERIOD_MS);
    }

}

void control_loop( void *parameters) {


    last_time = this_time;
    this_time = micros();
    dt = (float)(this_time - last_time)/one_second;

    //Update Metrics Ect
    //Use 50% Low Pass Filter, Average Last Two Cycles
    if ( !continous_active || !thermally_ready ){
        instantnious_cycle_time = 1E9;
        instantnious_production_rate=0;}
    else{
      instantnious_production_rate = 0.5 * (3600.0 / instantnious_cycle_time)
                                    + 0.5 * instantnious_production_rate;
    }


    //Use Rolling Integral
    past_hour_production_rate += (instantnious_production_rate
                                  - past_hour_production_rate)*(dt/1585.0);
                                  //+ past_hour_production_rate * (1.0-dt/3600.0);

    temp_past_minute += (temperature_former-temp_past_minute)*(dt/26.4);
                        //+ temp_past_minute * (1.0 - dt/60.0);

    last_count = cumulative_cycles;

    //Check Readyness
    if (thermally_ready){
      if ( temp_past_minute > PROCESS_TEMP - 1.0 ){ //Hysterisis Value
        thermally_ready = true; }
      else{ thermally_ready = false; }
    }
    else{
      if ( temp_past_minute >= PROCESS_TEMP ){
        thermally_ready = true; }
      else{ thermally_ready = false; }
    }


    bool double_clicked = action_button -> doubleClicked();

    switch(current_mode){
      case  FEED_BACK:
        if (piston_closed == true){ open_piston();}
        if ( ACTION_STATE && !double_clicked)
          while ( ACTION_STATE ){ feed_back_continuous();}
        else if (double_clicked){  feed_back_one_unit(); }
        //else{ feed_stop(); }
        break;
      case FEED_FORWARD:
        if (piston_closed == true){ open_piston();}
        if ( ACTION_STATE && !double_clicked )
          while ( ACTION_STATE ){ feed_forward_continuous(); }
        else if ( double_clicked ){ feed_forward_one_unit(); }
        //else{ feed_stop(); }
        break;
      case IDLE:
        if (action_button -> holding()){
          if (piston_closed == true){ open_piston(); }
          cycle_once();
        }
        break;
      case CONTINUOUS:
        if(continous_active){
          if (thermally_ready){
            //Feed Forward
            if (piston_closed == true){ open_piston();}
            cycle_once();
          }{
            //Close Piston//
            close_piston();
          }
        }
        else{
          Serial.print(".");
          vTaskDelay((TickType_t) 10 / portTICK_PERIOD_MS);
        }
        break;
    }
    vTaskDelay(0);
  //}

}

void setup() {
  Serial.begin(115200);

  disableCore0WDT();
  disableCore1WDT();

  begin_server();

  ledcSetup(therm_ledChannel, therm_freq, therm_resolution);
  ledcAttachPin(HEATER_PIN_FORMER, therm_ledChannel);
  ledcSetup(therm_ledChannel+1, therm_freq, therm_resolution);
  ledcAttachPin(HEATER_PIN_SMASHER, therm_ledChannel+1);
  // ledcSetup(step_ledChannel, step_freq, step_resolution);
  // ledcAttachPin(STEP_PIN, step_ledChannel);


  pinMode(STEP_PIN,OUTPUT);
  pinMode(DIR_PIN,OUTPUT);
  pinMode(PNEUMATIC_SOLENOID_PIN,OUTPUT);
  pinMode(EXTRA_GPIO,OUTPUT);

  stepper.begin(100,1);
  stepper.setSpeedProfile(stepper.LINEAR_SPEED,500,5000);

  //Start Sensor Task
  xTaskCreate(
                      read_thermocouples,          /* Task function. */
                      "Read Thermocouples",        /* String with name of task. */
                      10000,            /* Stack size in bytes. */
                      NULL,             /* Parameter passed as input of the task */
                      3,                /* Priority of the task. */
                      NULL);            /* Task handle. */



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

  // //Motion Control Tasks
  // xTaskCreate(
  //                   feed_control,          /* Task function. */
  //                   "Feed Control",        /* String with name of task. */
  //                   10000,            /* Stack size in bytes. */
  //                   NULL,             /* Parameter passed as input of the task */
  //                   3,                /* Priority of the task. */
  //                   NULL);            /* Task handle. */


  //User Input And Meta
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

  // xTaskCreate(     control_loop,          /* Task function. */
  //                   "Control Loop ",        /* String with name of task. */
  //                   10000,            /* Stack size in bytes. */
  //                   NULL,             /* Parameter passed as input of the task */
  //                   2,                /* Priority of the task. */
  //                   NULL);            /* Task handle. */

}

void loop(){
  control_loop( NULL );
  delay(1);
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


// xTaskCreate(     wifi_status_task,          /* Task function. */
//                   "Wifi Status Server",        /* String with name of task. */
//                   10000,            /* Stack size in bytes. */
//                   NULL,             /* Parameter passed as input of the task */
//                   6,                /* Priority of the task. */
//                   NULL);            /* Task handle. */
