#ifndef PIN_ASSIGNMENTS_H
#define PIN_ASSIGNMENTS_H

//Linear CMOS Sensor Pins
#define CMOS_CLK 25
#define ADC_CLK 24
#define EOS 20
#define ST 6
#define LIGHT_IN 0
#define TRIG_OUT 26

//Fan
#define FAN_OUT 9

//CAN
#define CAN_STDBY1 2 
#define CAN_STDBY2 32
#define CAN_CTX1 22
#define CAN_CTX2 30
#define CAN_CRX1 23
#define CAN_CRX2 31
#define CAN_SW1 15
#define CAN_SW2 16

//Limit Switches
#define LIMIT_SW1 7     
#define LIMIT_SW2 8

//Time of Flight
#define TOF_SCL 19
#define TOF_SDA 18
#define TOF_GPID 17 

//Laser Diode
#define LASER_OUT 33

//Servos
#define SERVO1 37
#define SERVO2 38

//Conceptual Sensor Pins
#define SCLK ADC_CLK//Redundant dont worry about it

#endif