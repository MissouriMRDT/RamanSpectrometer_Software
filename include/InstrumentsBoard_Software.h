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

RoveVNH InstrumentGantryMotor(PWM, FWD, RVS, CS);
RoveJoint InstrumentGantry(&InstrumentGantryMotor);

LimitSwitch forwardLimit(FWD_LIM);
LimitSwitch reverseLimit(RVS_LIM);

ILX511 RamanCCD(CCD_CLK_OUT, CCD_CLK_IN, CCD_ROG, CCD_VOUT);

// Watchdog
#define WATCHDOG_TIMEOUT 300000
IntervalTimer Watchdog;
uint8_t watchdogStatus = 0;
uint8_t watchdogOverride = 0;

void feedWatchdog();
void estop();

RoveCommEthernet RoveComm;

#endif /* INSTRUMENTSBOARD_SOFTWARE_H */