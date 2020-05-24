#include "thermal_control.h"
#include "analogWrite.h"

bool thermally_ready = false;

int therm_freq = 1000;
int therm_ledChannel = 1;
int therm_resolution = 8;

float pid_Kf_p = 150.0;
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

int smasher_pwm_bias;

void former_heater_pid(){
  pid_former_error = (Temperature_Goal_Former + 0.5) - temperature_former;

  if (current_pwm_former <= PWM_MAX && heat_on){
    //4x divider to delay action
    pid_former_ei += pid_former_error * K_I * ((float)(pid_interval_ms)/1000.0);
    pid_former_ei = max((float)PWM_MIN,min(pid_former_ei, (float)PWM_MAX));
  }

  if( heat_on){
    current_pwm_former = (int) max((float)PWM_MIN,min(K_P * pid_former_error
                                    + pid_former_ei,(float) PWM_MAX));
  }
  else{
    current_pwm_former = 0;
  }
  ledcWrite(therm_ledChannel, current_pwm_former);
}

void smasher_heater_pid(){
  //pid_smasher_error = Temperature_Goal_Smasher - temperature_smasher;
  pid_smasher_error = (Temperature_Goal_Smasher + 0.5) - temperature_former;

  if (current_pwm_smasher <= PWM_MAX && heat_on){
    //4x divider to delay action
    pid_smasher_ei += pid_smasher_error * K_I * ((float)(pid_interval_ms)/1000.0);
    pid_smasher_ei = max((float)PWM_MIN,min(pid_smasher_ei, (float) PWM_MAX));
  }


  if( heat_on){
    current_pwm_smasher = (int) max((float)PWM_MIN,min(K_P * pid_smasher_error
                                          + pid_smasher_ei+smasher_pwm_bias,(float) PWM_MAX));
  }
  else{
    current_pwm_smasher = 0;
  }
  ledcWrite(therm_ledChannel+1, current_pwm_smasher);
}

void former_control_task(void * parameter){

  const TickType_t xDelay = pid_interval_ms / portTICK_PERIOD_MS;

  pinMode(HEATER_PIN_FORMER, OUTPUT);

  for(;;){ // Always Run
    former_heater_pid();

    vTaskDelay( xDelay );
  }
  vTaskDelete( NULL );
}

void smasher_control_task(void * parameter){

  const TickType_t xDelay = pid_interval_ms / portTICK_PERIOD_MS;
  smasher_pwm_bias = 0;
  pinMode(HEATER_PIN_SMASHER, OUTPUT);

  for(;;){ // Always Run
    smasher_heater_pid();

    vTaskDelay( xDelay );
  }
  vTaskDelete( NULL );
}
