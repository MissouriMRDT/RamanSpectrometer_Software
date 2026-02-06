#include "Sensor.h"


Sensor::Sensor()
{
    pinMode(SS, OUTPUT);
    SPI.begin();

    CMOSToggle = false; //not sure this should be the start state. 

    spiSettings = SPISettings(ADC_CLK_SPEED, MSBFIRST, SPI_MODE0);
    CMOSTimer.begin(stepCMOS, 1 / (CMOS_CLK_SPEED / 2));
}


bool Sensor::stepCMOS()
{
    CMOSToggle = !CMOSToggle; 
}