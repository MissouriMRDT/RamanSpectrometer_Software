#include <SPI.h>
#include <Arduino.h>
#include "Pin_Assignments.h"
#include "SPIBetter.h"

/*
CMOS reading info:

Rerout trig to the EOS pin so we can use it for data retrieval
 -- On trig low begin SPI data retrieval (14bits)

Run CMOS at 900KHz to allow for sufficient conversion and pixel input time.

Run ADC at 100MHZ? as long as the data would be able to transfer within 1us
*/

#define ADC_CLK_SPEED 50000000
#define CMOS_CLK_SPEED 10000 //900000 / 2
#define SELECTION_STATE HIGH //im doing this because the SPI documentation says LOW, but the ADC documentation says HIGH
#define TRIG_OVER 89

class Sensor
{
public:
    Sensor();
    ~Sensor();

    static void read(bool);
    void cancelData();
    uint16_t* getData();
    void setMode(uint8_t);
    void setStartCycles(int);
    void setRepeats(uint16_t);
    static void clearData();
    static void clearBackground();
private:
    static void stepCMOS();
    static void ADCReceive();

    static SPISettings spiSettings;

    static IntervalTimer CMOSTimer;
    //Volatile because i dont want read() call to mess up anything once it enters interupt or timer.
    static volatile bool CMOSToggle; 
    static volatile uint32_t CMOSHalfCycles;
    static volatile uint32_t CMOSTStartCycles;
    static volatile uint32_t shutDownCount;
    static volatile uint32_t integrationCount;
    static uint8_t mode; 
    static volatile uint16_t repeats;
    static uint16_t repeatAmount;
    
    static uint16_t adcData[PIXEL_COUNT];
    static uint16_t backgroundData[PIXEL_COUNT];
    static volatile uint32_t adcDataCount;

    static volatile bool dataState;
    static volatile bool errorSwitch;
}; 