#ifndef CONFIG_H
#define CONFIG_H


const int  MAXDO = 19;
const int  MAXCLK = 18;

const int MAXCS_FORMER = 4;
const int MAXCS_SMASHER = 17;

const int HEATER_PIN_FORMER = 25;
const int HEATER_PIN_SMASHER  = 26;

const int STEP_PIN = 33;
const int DIR_PIN  = 32;

const int PNEUMATIC_SOLENOID_PIN = 21;
const int EXTRA_GPIO = 3;

const int I2C_CLK =22;
//Converted due to tx pin / serial conflict
//const int I2C_SDA =21;

const int IO_BUTTON_1= 12; //MODE
const int IO_BUTTON_2 =13; //Not Connected
const int IO_BUTTON_3 =27; //Action
const int IO_BUTTON_4 =14; //FEED

//PWM Filter Gains
#define PWM_MAX 252
#define PWM_MIN 0

#endif
