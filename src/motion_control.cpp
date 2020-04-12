#include "motion_control.h"
#include "esp_task_wdt.h"

hw_timer_t * timer = NULL;

A4988 stepper(MOTOR_STEPS, DIR_PIN, STEP_PIN);

bool piston_closed = false;
bool do_feeding = false;

float feed_distance = 25.0; //mm
float feed_diameter = 14.5;
float pi = 3.1415;
float feed_circumfrence = feed_diameter * pi;

float steps_per_rotation = 200;

float distance_per_step = (pi * feed_diameter ) / steps_per_rotation;
int steps_per_feed = (int) feed_distance * 2.0 / distance_per_step;

float feed_degrees_rotation = 360.0 * (feed_circumfrence / feed_distance);

long steps_to_go = 0;
bool step_state;

// setting PWM properties
int step_freq = 700;
int step_ledChannel = 5;
int step_resolution = 8;

float meters_fed_while_on;
float meters_fed_last_hour;
float current_meters_per_minute;


void feed_control(void * parameters ){

    pinMode(STEP_PIN,OUTPUT);
    pinMode(DIR_PIN,OUTPUT);

    long next_timer_interval;
    //digitalWrite(DIR_PIN,LOW);

    while(1){ // Always Run
      //if (stepper.getCurrentState() > stepper.STOPPED){
      next_timer_interval = stepper.nextAction();
      //if(next_timer_interval == 0){ stepper.stop(); }
      //}
      vTaskDelay(0);
    }
}

void feed_stop(){
  Serial.println("Feed Stop");
  stepper.stop();
}

void feed_forward_one_unit(){
  Serial.println("Feed Forward");
  //if (stepper.getCurrentState() == stepper.STOPPED){
  stepper.move(STEP_LENGTH);
  //}
}

void feed_forward_continuous(){
  Serial.print(">");
  //if (stepper.getCurrentState() == stepper.STOPPED){
  stepper.move(STEP_LENGTH/10);
  //}
}

void feed_back_continuous(){
  Serial.print("<");
  //if (stepper.getCurrentState() == stepper.STOPPED){
  stepper.move(-STEP_LENGTH/10);
  //}
}

void feed_back_one_unit(){
  Serial.println("Feed Back");
  //if (stepper.getCurrentState() == stepper.STOPPED){
  stepper.move(-STEP_LENGTH);
  //}

}

void close_piston(){
  Serial.println("Close");
  digitalWrite(PNEUMATIC_SOLENOID_PIN,HIGH);
  vTaskDelay((TickType_t) PISTON_TIME / portTICK_PERIOD_MS);
  piston_closed = true;
}

void open_piston(){
  Serial.println("Open");
  digitalWrite(PNEUMATIC_SOLENOID_PIN,LOW);
  vTaskDelay((TickType_t) PISTON_TIME / portTICK_PERIOD_MS);
  piston_closed = false;
}

void pop_pison(){
  Serial.println("Open");
  digitalWrite(PNEUMATIC_SOLENOID_PIN,LOW);
  vTaskDelay((TickType_t) DELAY_TIME / portTICK_PERIOD_MS);
  piston_closed = false;
}

void cycle_once(){
    Serial.println("Cycle Once");

    unsigned long cycle_start = micros();

    pop_pison();

    feed_forward_one_unit();
    vTaskDelay((TickType_t) FEED_TIME / portTICK_PERIOD_MS);
    //Close
    close_piston();
    //Wait
    vTaskDelay((TickType_t) CLOSE_TIME / portTICK_PERIOD_MS);
    //smart_delay_micros((uint32_t) CLOSE_TIME*1000);
    //Open
    open_piston();
    //Eject
    vTaskDelay((TickType_t) EJECT_TIME / portTICK_PERIOD_MS);
    //smart_delay_micros((uint32_t) EJECT_TIME*1000);
    unsigned long cycle_end = micros();

    //Well use the average of this time, and the last time in a rolling low_pass filter
    instantnious_cycle_time = 0.5 * (cycle_end - cycle_start)/one_second + instantnious_cycle_time * 0.5;
    cumulative_cycles += 1;
}
