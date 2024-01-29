#ifndef RAMAN_SOFTWARE_H
#define RAMAN_SOFTWARE_H

#include "PinAssignments.h"

#include <stdint.h>

#include "RoveComm.h"


// RoveComm
EthernetServer TCPServer(RC_ROVECOMM_ETHERNET_TCP_PORT);
RoveCommEthernet RoveComm;


#define BAUDRATE 115200

#define CLK_FREQUENCY (1000000.0/5.0) //Hz
#define CLK_TIMEPERIOD (1.0 / CLK_FREQUENCY) //seconds

#define CLK_t5 (3000)
#define ROG_t7 (5000-50) //ns
#define CLK_t9 (3000)

#define PIXEL_COUNT 2048
#define CLK_REPETITIONS 2088


uint16_t pixelArray[PIXEL_COUNT];

void initADC();
void CLK_ISR();

#endif