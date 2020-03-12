#include "motion_control.h"
#include "esp_task_wdt.h"


A4988 stepper(MOTOR_STEPS, DIR_PIN, STEP_PIN);

bool piston_closed = false;
bool do_feeding = false;

float feed_distance = 25.0; //mm
float feed_diameter = 14.5;
float pi = 3.1415;
float feed_circumfrence = feed_diameter * pi;

float steps_per_rotation = 200;

float distance_per_step = (pi * feed_diameter ) / steps_per_rotation;
int steps_per_feed = (int) feed_distance  *2.0 / distance_per_step;

float feed_degrees_rotation = 360.0 * (feed_circumfrence / feed_distance);

long steps_to_go = 0;
bool step_state;

void piston_control(void * parameters ){
    const TickType_t xDelay = piston_interval_ms / portTICK_PERIOD_MS;

    pinMode(PNEUMATIC_SOLENOID_PIN,OUTPUT);

    while(1){ // Always Run

       if (piston_closed){

         digitalWrite(PNEUMATIC_SOLENOID_PIN,HIGH);

       }
       else{

         digitalWrite(PNEUMATIC_SOLENOID_PIN,LOW);

       }


      vTaskDelay(xDelay);
    }
}

void feed_control(void * parameters ){

    pinMode(STEP_PIN,OUTPUT);
    pinMode(DIR_PIN,OUTPUT);

    //stepper.begin(1,10);

    //digitalWrite(DIR_PIN,LOW);

    while(1){ // Always Run
    //
    //   if (do_feeding){ //IMplement A4988 driver using async principals.
    //     steps_to_go = steps_per_feed;
    //     esp_task_wdt_reset();
    //     if (steps_to_go > 0){
    //         while(steps_to_go>0){
    //           esp_task_wdt_reset();
    //           step_state != step_state;
    //           digitalWrite(STEP_PIN,step_state);
    //           steps_to_go = steps_to_go - 1;
    //           //vTaskDelay(xDelay);
    //         }
    //     }
    //     do_feeding = false;
    //     esp_task_wdt_reset();
    //   }
    }
}
