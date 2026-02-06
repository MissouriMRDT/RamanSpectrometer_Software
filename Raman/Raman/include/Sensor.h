#include <SPI.h>
#include <Arduino.h>
#include "Pin_Assignments.h"

/*
CMOS reading info:

Rerout trig to the EOS pin so we can use it for data retrieval
 -- On trig low begin SPI data retrieval (14bits)

Run CMOS at 900KHz to allow for sufficient conversion and pixel input time.

Run ADC at 100MHZ? as long as the data would be able to transfer within 1us
*/

#define ADC_CLK_SPEED 100000000
#define CMOS_CLK_SPEED 900000
#define SELECTION_STATE HIGH //im doing this because the SPI documentation says LOW, but the ADC documentation says HIGH
#define START_CYCLE 1
#define PIXEL_COUNT 2004
#define TRIG_OVER 89

class Sensor
{
public:
    Sensor();

    void read();
    uint16_t* getData();
private:
    static void stepCMOS();
    static void ADCReceive();

    static SPISettings spiSettings;

    static IntervalTimer CMOSTimer;
    static bool CMOSToggle; 
    static uint8_t CMOSHalfCycles;
    
    static uint16_t *adcData;
    static uint16_t adcDataCount;

    static bool dataState;
}