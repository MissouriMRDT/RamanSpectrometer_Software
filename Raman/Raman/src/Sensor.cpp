#include "Sensor.h"


SPISettings Sensor::spiSettings;

IntervalTimer Sensor::CMOSTimer;
bool Sensor::CMOSToggle;
uint8_t Sensor::CMOSHalfCycles;

uint16_t* Sensor::adcData;
uint16_t Sensor::adcDataCount;

bool Sensor::dataState;


Sensor::Sensor()
{
    /*adcData = new uint16_t[PIXEL_COUNT];
    spiSettings = SPISettings(ADC_CLK_SPEED, MSBFIRST, SPI_MODE0);
    */
    dataState = false;
    Serial.println("BAD");
}


Sensor::~Sensor()
{
    delete adcData; 
}


void Sensor::read()
{
    pinMode(SS, OUTPUT);
    pinMode(CMOS_CLK, OUTPUT);
    pinMode(ST, OUTPUT);
    digitalWrite(ST, HIGH);
    SPI.begin();

    adcDataCount = 0;

    dataState = false;

    CMOSHalfCycles = 0;
    CMOSToggle = false; //not sure this should be the start state. 

    CMOSTimer.begin(stepCMOS, 1000000. / (CMOS_CLK_SPEED / 2.));
}


void Sensor::stepCMOS()
{
    CMOSToggle = !CMOSToggle; 

    digitalWriteFast(CMOS_CLK, CMOSToggle ? HIGH : LOW);
    
    //Start-time Done
    if (CMOSHalfCycles == START_CYCLE)
    {
        digitalWriteFast(ST, LOW);
        //Begin the SPI stuff here because it needs to feed SCLK
        
        digitalWrite(SS, SELECTION_STATE);
        SPI.beginTransaction(spiSettings);

        //Setup the interupt so checking for the trig pin
        attachInterrupt(EOS, ADCReceive, FALLING);//MAKE SURE TO DETACH
    }

    if (CMOSHalfCycles <= START_CYCLE)
        CMOSHalfCycles++;
}


void Sensor::ADCReceive()
{
    //This accounts for the CMOS's integration time which is TRIG_OVER trig cycles after ST pulled low
    if (adcDataCount > TRIG_OVER)
    {
        //ADC outputs 14 bits so shift it left 2 so that its scaled correctly
        adcData[adcDataCount] = (SPI.transfer16(0) >> 2);

        //Check for final pixel. if end deinitialize sensor systems
        if (adcDataCount == PIXEL_COUNT + TRIG_OVER) // + TRIG_OVER because we are PIXEL_COUNT after the integration time
        {
            SPI.endTransaction();
            detachInterrupt(EOS);
            digitalWrite(SS, !SELECTION_STATE);
            CMOSTimer.end();
            dataState = true;
        }
    }

    adcDataCount++;
}


uint16_t* Sensor::getData()
{
    //return nullptr if the data is incomplete
    if (dataState)
        return adcData;
    else
        return nullptr;
}