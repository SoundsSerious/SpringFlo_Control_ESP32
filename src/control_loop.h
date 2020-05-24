#include "Arduino.h"
#include "common.h"
#include "config.h"
#include "cs_timing.h"
#include "sensors.h"
#include "thermal_control.h"
#include "motion_control.h"
#include "user_input.h"


unsigned long last_time;
unsigned long this_time;
float dt;
unsigned long last_count;



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

    //Check Readyness - 1.5 Deg Hysterisis
    if (thermally_ready){
      if ( temp_past_minute > PROCESS_TEMP - 2.5 ){ //Hysterisis Value
        thermally_ready = true; }
      else{ thermally_ready = false; }
    }
    else{
      if ( temp_past_minute >= PROCESS_TEMP ){
        thermally_ready = true; }
      else{ thermally_ready = false; }
    }


    bool double_clicked = action_button -> doubleClicked();
    //bool double_clicked = false;

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
            //close_piston();
            open_piston(); //Lets keep the pison open so the heat transfer reset is the same
          }
        }
        else{
          Serial.print(".");
          vTaskDelay((TickType_t) control_loop_ms / portTICK_PERIOD_MS);
        }
        break;
    }
    vTaskDelay((TickType_t) control_loop_ms / portTICK_PERIOD_MS);
};



void control_loop_task(void * parameters){

  for(;;){
    control_loop( NULL );
  }
  vTaskDelete( NULL );

};
