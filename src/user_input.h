
#ifndef USER_INPUT_H
#define USER_INPUT_H

#include "Arduino.h"
#include "config.h"



bool check_pin(int button_num){
  bool val = (bool) digitalRead(button_num);
  if (val == true){Serial.print("Button");Serial.print(button_num);Serial.println("Pressed");return true;}
  return false;
}


void setup_user_input(){
  pinMode(IO_BUTTON_1,INPUT);
  pinMode(IO_BUTTON_2,INPUT);
  pinMode(IO_BUTTON_3,INPUT);
  pinMode(IO_BUTTON_4,INPUT);
}

void check_user_input(){
  check_pin(IO_BUTTON_1);
  check_pin(IO_BUTTON_2);
  check_pin(IO_BUTTON_3);
  check_pin(IO_BUTTON_4);
}


#endif
