#ifndef RAMAN_SOFTWARE_H
#define RAMAN_SOFTWARE_H

#include "Pin_Assignments.h"


#include <Arduino.h>

#include "RoveComm.h"
#include "S16514.h"
#include "Smoco.h"

//For TOF
#include <Wire.h>
#include "vl53l4cx_class.h"
#include <cfloat>

//RoveComm
RoveCommEthernet roveComm;
RoveCommPacket packet;

//Cmos
S16514 linearSensor;

//Gantry Motor
#define ATAN_T4_CAN ACAN_T4::can1

ACAN_T4_Settings acanSettings(125 * 1000);
Smoco smoco(&ATAN_T4_CAN, 0x00);
int16_t instrumentGantrySpeed = 0;

//Telemetry
float telemetryCounter;

//TOF
VL53L4CX tofSensor(&Wire, A1); 
byte identifyTofAddress();


// Watchdog
#define WATCHDOG_TIMEOUT 300000
IntervalTimer Watchdog;
uint8_t watchdogStatus = 0;
uint8_t watchdogOverride = 0;

void feedWatchdog();
void estop();

#endif