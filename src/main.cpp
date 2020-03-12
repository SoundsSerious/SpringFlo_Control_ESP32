#include "Arduino.h"
#include <WiFi.h>
#include "analogWrite.h"


#include "config.h"
#include "cs_timing.h"
#include "sensors.h"
#include "thermal_control.h"
#include "motion_control.h"
#include "user_input.h"

// Set web server port number to 80
WiFiServer server(80);

bool thermally_ready = false;

void begin_server(){
// Connect to Wi-Fi network with SSID and password

// Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println("SpaceCase");
  WiFi.begin("SpaceCase", "9252623793");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void setup() {
  Serial.begin(115200);
  begin_server();

  //setup_motion_control();
  setup_user_input();

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

  //Initial Feed
  steps_to_go = 1000;
  if (steps_to_go > 0){
      while(steps_to_go>0){
        step_state = !step_state;
        digitalWrite(STEP_PIN,step_state);
        delay(2);
        steps_to_go = steps_to_go - 1;
      }
  }
}

void print_thermal_status_wifi(){

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {
      while (client.connected()) {            // loop while the client's connected
        if (client.available()) {             // if there's bytes to read from the client,
          char c = client.read();
        }
      }

      client.print(Temperature_Goal_Former);
      client.print(" ");
      client.print(temp_raw_former);
      client.print(" ");
      client.print(temperature_former);
      client.print(" | ");

      client.print(pid_former_error*100.0 /255);
      client.print(" ");
      client.print(pid_former_ei*100.0 /255);
      client.print(" ");
      client.print(current_pwm_former*100.0 /255);
      client.print(" | ");

      client.print(pid_smasher_error*100.0 /255);
      client.print(" ");
      client.print(pid_smasher_ei*100.0 /255);
      client.print(" ");
      client.print(current_pwm_smasher*100.0 /255);
      client.print(" | ");

      client.print(" | ");
      client.print(former_sensor_is_nominal);
      client.print(" ");
      client.print(thermally_ready);
      client.print(" ");
      client.print("|");

      client.print(" | ");
      client.print(do_feeding);
      client.print(" ");
      client.print(piston_closed);
      client.print(" ");
      client.println("|");
 }
 client.stop();
}

void print_thermal_status(){
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

}

void loop() {

  print_thermal_status();
  print_thermal_status_wifi();

  if ( (temperature_smasher > Temperature_Goal_Smasher-5.0) ){
    thermally_ready = true;
  }
  else{
    thermally_ready = false;

    //Close Piston//
    digitalWrite(PNEUMATIC_SOLENOID_PIN,HIGH);

    delay(1000);
  }

  if (thermally_ready){

    delay(100);


    //Feed Forward
    //Serial.println("Feed");
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

    delay(1000);

    //Close
    //Serial.println("Close");
    digitalWrite(PNEUMATIC_SOLENOID_PIN,HIGH);

    //Wait
    delay(2000);

    //Open
    //Serial.println("Open");
    digitalWrite(PNEUMATIC_SOLENOID_PIN,LOW);

    //Eject
    delay(500);
  }
  else {
    // digitalWrite(PNEUMATIC_SOLENOID_PIN,LOW);
    // delay(1000);
  }

}



// current_time_ms = millis();

//analogWrite(HEATER_PIN_SMASHER, 128);

// if ((current_time_ms - last_sensor_update) > sensor_interval_ms)
// {
//   read_sensors();
//   print_thermal_status();
//   last_sensor_update = current_time_ms;
// }
//
// if ((current_time_ms - last_pid_update_ms) > pid_interval_ms)
// {
//   update_thermal_control();
//
//   // analogWrite(HEATER_PIN_SMASHER, current_pwm_smasher);
//   // analogWrite(HEATER_PIN_FORMER, current_pwm_former);
//
//   last_pid_update_ms = current_time_ms;
// }

// if ((current_time_ms - last_step_update_ms) > step_interval_ms)
// {
//   step_state = !step_state;
//   digitalWrite(STEP_PIN,step_state);
//   last_step_update_ms = current_time_ms;
// }
//
// if ((current_time_ms - last_userinput_interval_ms) > userinput_interval_ms)
// {
//   check_user_input();
//   last_userinput_interval_ms = current_time_ms;
// }
