#include "Arduino.h"
#include <WiFi.h>
//#include "analogWrite.h"
#include <WiFi.h>

#include "common.h"
#include "config.h"
#include "cs_timing.h"
#include "sensors.h"
#include "thermal_control.h"
#include "motion_control.h"
#include "user_input.h"
#include "wifi_status_server.h"
#include "serial_statements.h"
#include "control_loop.h"


void setup() {

 Serial.begin(115200);
  //
  //disableCore0WDT();
  //disableCore1WDT();

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
  delay( 100);
  xTaskCreate(
                      read_thermocouples,          /* Task function. */
                      "Read Thermocouples",        /* String with name of task. */
                      10000,            /* Stack size in bytes. */
                      NULL,             /* Parameter passed as input of the task */
                      3,                /* Priority of the task. */
                      NULL);            /* Task handle. */



  //Start Thermal Control Tasks
  delay( 100);
  xTaskCreate(
                      smasher_control_task,          /* Task function. */
                      "Smasher Thermal Control",        /* String with name of task. */
                      10000,            /* Stack size in bytes. */
                      NULL,             /* Parameter passed as input of the task */
                      2,                /* Priority of the task. */
                      NULL);            /* Task handle. */
  delay( 100);
  xTaskCreate(
                    former_control_task,          /* Task function. */
                    "Former Thermal Control",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    2,                /* Priority of the task. */
                    NULL);            /* Task handle. */



  delay( 100);
  //User Input And Meta
  xTaskCreate(     check_user_input,          /* Task function. */
                    "Check user input",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
  //

  delay( 100);
  xTaskCreate(     print_status,          /* Task function. */
                    "Serial Status ",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    20,                /* Priority of the task. */
                    NULL);            /* Task handle. */

  delay( 100);
  xTaskCreate(     control_loop_task,          /* Task function. */
                    "Control Loop ",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */

 //Serial.println("Beginning Server");
  //TickType_t xdelay = 500/ portTICK_PERIOD_MS;
  delay( 1000);
  WiFi.mode(WIFI_STA);
  delay( 100);
  WiFi.begin(ssid, password);
  delay( 100);
  while (WiFi.status() != WL_CONNECTED) {
    delay( 500);

   //Serial.println("Connecting...");
  }
  delay( 100);
 //Serial.println("Connected");
  delay( 100);
  begin_server();
}



void loop(){
  vTaskDelay((TickType_t) DELAY_TIME / portTICK_PERIOD_MS);
}

// //Motion Control Tasks
// xTaskCreate(
//                   feed_control,          /* Task function. */
//                   "Feed Control",        /* String with name of task. */
//                   10000,            /* Stack size in bytes. */
//                   NULL,             /* Parameter passed as input of the task */
//                   3,                /* Priority of the task. */
//                   NULL);            /* Task handle. */

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
