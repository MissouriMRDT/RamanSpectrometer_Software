#ifndef INSTRUMENTSBOARD_SOFTWARE_H
#define INSTRUMENTSBOARD_SOFTWARE_H

#include "PinAssignments.h"
#include <stdint.h>
#include <RoveComm.h>
#include <RoveVNH.h>
#include <RoveJoint.h>
#include <RoveEncoder.h>
#include <LimitSwitch.h>
#include "ILX511.h"
#include "PWMServo.h"

RoveVNH InstrumentGantryMotor(PWM, FWD, RVS, CS);
RoveJoint InstrumentGantry(&InstrumentGantryMotor);

LimitSwitch forwardLimit(LIMIT_SWITCH_1);
LimitSwitch reverseLimit(LIMIT_SWITCH_2);

ILX511 RamanCCD(CCD_CLK, CCD_ROG, CCD_VOUT);


#define PITCH_UPPER_LIMIT 150
#define PITCH_LOWER_LIMIT 50
#define ROLL_UPPER_LIMIT  170
#define ROLL_LOWER_LIMIT  60
PWMServo rollServo;
PWMServo pitchServo;
uint16_t targetPitch = 180;
uint16_t targetRoll = 180;

// Watchdog
#define WATCHDOG_TIMEOUT 300000
IntervalTimer Watchdog;
uint8_t watchdogStatus = 0;
uint8_t watchdogOverride = 0;
int16_t targetSpeed = 0;

void feedWatchdog();
void estop();

RoveCommEthernet RoveComm;

#endif /* INSTRUMENTSBOARD_SOFTWARE_H */