#include "user_input.h"


Button* mode_button = NULL;
Button* feed_button = NULL;
Button* action_button = NULL;

bool MODE_STATE = false;
bool FEED_STATE = false;
bool ACTION_STATE = false;

void setup_user_input(){
  pinMode(IO_BUTTON_1,INPUT);
  pinMode(IO_BUTTON_2,INPUT);
  pinMode(IO_BUTTON_3,INPUT);
  pinMode(IO_BUTTON_4,INPUT);

  // mode_button = new Button(IO_BUTTON_1);
  // feed_button = new Button(IO_BUTTON_4);
  action_button = new Button(IO_BUTTON_3);

}

void check_user_input( void *parameters){

  setup_user_input();

  const TickType_t xDelay = userinput_interval_ms / portTICK_PERIOD_MS;

  while(1){ // Always Run
    // mode_button -> update();
    // feed_button -> update();

    action_button -> update();

    ACTION_STATE = check_pin(IO_BUTTON_3);
    MODE_STATE = check_pin(IO_BUTTON_1);
    FEED_STATE = check_pin(IO_BUTTON_4);
    //Check State Of System
    if(MODE_STATE){ //We're High this is the operational mode
      if(FEED_STATE){
        if(action_button -> clicked()){
          continous_active = !continous_active;
        }
        current_mode = CONTINUOUS;
      }else{
        current_mode = IDLE;
      }
    }
    else{ //We're Low -> this should be feed forward / back
      if(FEED_STATE){
        current_mode = FEED_FORWARD;
      }else{
        current_mode = FEED_BACK;
      }
    }

    vTaskDelay(xDelay);
  }
}

bool check_pin(int button_num){
  bool val = (bool) digitalRead(button_num);
  if (val == true){/*Serial.print("Button");Serial.print(button_num);Serial.println("Pressed")*/;return true;}
  return false;
}
