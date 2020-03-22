
#ifndef USER_INPUT_H
#define USER_INPUT_H

#include "Arduino.h"
#include "config.h"
#include "cs_timing.h"
#include "common.h"

#include <SimpleButton.h>

using namespace simplebutton;

extern Button* mode_button;
extern Button* feed_button;
extern Button* action_button;

extern bool MODE_STATE;
extern bool FEED_STATE;
extern bool ACTION_STATE;

void setup_user_input();

void check_user_input(void * parameter);

bool check_pin(int button_num);
#endif
