#include <SPI.h>
#include <Arduino.h>
#include "Pin_Assignments.h"

#define ADC_CLK_SPEED 60000000
#define CMOS_CLK_SPEED 10000000
#define SELECTION_STATE HIGH //im doing this because the SPI documentation says LOW, but the ADC documentation says HIGH

class Sensor
{
public:
    Sensor();

    static bool stepCMOS();

private:
    static SPISettings spiSettings;

    static IntervalTimer CMOSTimer;
    static bool CMOSToggle; 
}