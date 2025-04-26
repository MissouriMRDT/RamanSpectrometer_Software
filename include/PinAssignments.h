#ifndef PINASSIGNMENTS_H
#define PINASSIGNMENTS_H

// Raman Pins
#define CCD_CLK 25
// #define CCD_CLK_IN 27 // mag wire
#define CCD_ROG 24
#define CCD_VOUT 26 // TODO: move to analog pin

#define GREEN_LASER 33

// Motor Pins
#define FWD  7
#define PWM  6
#define CS   20
#define RVS  4
#define SEL  3

// Button Pins
#define SW1 40
#define SW2 41

// Gimbal Pins
#define GIMBAL_PWM_A 36
#define GIMBAL_PWM_B 37

// Fan Pin
#define FAN_OUT 9

// Limit Switches
#define LIMIT_SWITCH_1 28
#define LIMIT_SWITCH_2 29
#define LIMIT_SWITCH_3 30
#define LIMIT_SWITCH_4 35

// Encoder Pins
#define ENCODER_A 22
#define ENCODER_B 23

// Time of Flight Sensor Pins
#define TOF_IO 17
#define TOF_SDA 18
#define TOF_SCL 19

// Strain Guage Pins
#define STRAIN_GUAGE_1 39
#define STRAIN_GUAGE_2 38

#endif /* PINASSIGNMENTS_H */