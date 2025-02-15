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

// Temperature Constants:
#define TEMP_ADC_MIN  31
#define TEMP_ADC_MAX  620
#define TEMP_MIN      -40
#define TEMP_MAX      125

RoveVNH InstrumentGantryMotor(PWM, FWD, RVS, CS);
RoveJoint InstrumentGantry(&InstrumentGantryMotor);

LimitSwitch forwardLimit(FWD_LIM);
LimitSwitch reverseLimit(RVS_LIM);

ILX511 RamanCCD(CCD_CLK_OUT, CCD_CLK_IN, CCD_ROG, CCD_VOUT);

EthernetServer TCPServer(RC_ROVECOMM_ETHERNET_TCP_PORT);
RoveCommEthernet RoveComm;

float analogMap(uint16_t measurement, uint16_t fromADC, uint16_t toADC, float fromAnalog, float toAnalog);

#endif /* INSTRUMENTSBOARD_SOFTWARE_H */