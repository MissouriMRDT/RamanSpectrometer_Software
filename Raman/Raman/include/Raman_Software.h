#ifndef RAMAN_SOFTWARE_H
#define RAMAN_SOFTWARE_H

#include "Pin_Assignments.h"

#include <Arduino.h>
#include "../lib/RoveComm_Arduino/src/RoveComm.h"
#include "S16514.h"

RoveCommEthernet roveComm;
RoveCommPacket packet;

S16514 linearSensor;

#endif