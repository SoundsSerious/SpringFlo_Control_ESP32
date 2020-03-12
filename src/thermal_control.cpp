#include "thermal_control.h"
#include "analogWrite.h"

float pid_Kf_p = 25.0;
float pid_Kf_i = 1.0;
float pid_Kf_d = 0.0;

// initialize the Former
float Temp_Goal = 130.0;
float Temperature_Goal_Former = Temp_Goal;
float Temperature_Goal_Smasher = Temp_Goal;

float pid_former_error, pid_former_ei, pid_former_ed;
float current_pwm_former;

float pid_smasher_error, pid_smasher_ei, pid_smasher_ed;
float current_pwm_smasher;

void former_heater_pid(){
  pid_former_error = Temperature_Goal_Former - temperature_former;

  if (current_pwm_former < PWM_MAX){
    //4x divider to delay action
    pid_former_ei += pid_former_error * ((float)(pid_interval_ms))/4000.0;
    pid_former_ei = max((float)PWM_MIN,min(pid_former_ei, (float)PWM_MAX));
  }

  if( former_sensor_is_nominal){
    current_pwm_former = (int) max((float)PWM_MIN,min(pid_Kf_p * pid_former_error + pid_Kf_i * pid_former_ei,(float) PWM_MAX));
    analogWrite(HEATER_PIN_FORMER, current_pwm_former);
  }
  else{
    current_pwm_former = 0;
    analogWrite(HEATER_PIN_FORMER, current_pwm_former);
  }
}

void smasher_heater_pid(){
  //pid_smasher_error = Temperature_Goal_Smasher - temperature_smasher;
  pid_smasher_error = Temperature_Goal_Smasher - temperature_former;

  if (current_pwm_smasher < PWM_MAX){
    //4x divider to delay action
    pid_smasher_ei += pid_smasher_error * ((float)(pid_interval_ms))/4000.0;
    pid_smasher_ei = max((float)PWM_MIN,min(pid_smasher_ei, (float) PWM_MAX));
  }


  if( former_sensor_is_nominal){
    current_pwm_smasher = (int) max((float)PWM_MIN,min(pid_Kf_p * pid_smasher_error + pid_Kf_i * pid_smasher_ei,(float) PWM_MAX));
    analogWrite(HEATER_PIN_SMASHER, current_pwm_smasher);
  }
  else{
    current_pwm_smasher = 0;
    analogWrite(HEATER_PIN_SMASHER, current_pwm_smasher);
  }
}

void former_control_task(void * parameter){

  const TickType_t xDelay = pid_interval_ms / portTICK_PERIOD_MS;

  pinMode(HEATER_PIN_FORMER, OUTPUT);

  while(1){ // Always Run
    former_heater_pid();

    vTaskDelay( xDelay );
  }
}

void smasher_control_task(void * parameter){

  const TickType_t xDelay = pid_interval_ms / portTICK_PERIOD_MS;

  pinMode(HEATER_PIN_SMASHER, OUTPUT);

  while(1){ // Always Run
    smasher_heater_pid();

    vTaskDelay( xDelay );
  }
}
