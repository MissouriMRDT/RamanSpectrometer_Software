#include "Sensor.h"


SPISettings Sensor::spiSettings;

IntervalTimer Sensor::CMOSTimer;
volatile bool Sensor::CMOSToggle;
volatile uint32_t Sensor::CMOSHalfCycles;
volatile uint32_t Sensor::CMOSTStartCycles;

volatile uint32_t Sensor::adcDataCount;

volatile bool Sensor::dataState;

uint16_t Sensor::adcData[];


Sensor::Sensor()
{
    pinMode(CMOS_CLK, OUTPUT);
    //digitalWrite(CMOS_CLK, HIGH);
    pinMode(ST, OUTPUT);
    //digitalWrite(ST, HIGH);
    dataState = false;
}


Sensor::~Sensor()
{
}


void Sensor::read()
{
    //TESTING TESTING TESTING TESTING
    /*pinMode(33, OUTPUT);
    digitalWrite(33, HIGH);*/
    pinMode(SS, OUTPUT);
    pinMode(TRIG_OUT, OUTPUT);
    pinMode(CMOS_CLK, OUTPUT);
    pinMode(ST, OUTPUT);
    digitalWrite(ST, HIGH);
    spiSettings = SPISettings(ADC_CLK_SPEED, MSBFIRST, SPI_MODE0);
    SPI.begin();
    

    adcDataCount = 0;

    dataState = false;

    CMOSHalfCycles = 0;
    CMOSToggle = false;

    //cli();

    //Serial.println("before");
    //delay(500);

    CMOSTimer.begin(stepCMOS, 1000000. / (CMOS_CLK_SPEED * 2));
    Serial.printf("Work: %f\n", 1000000. / (CMOS_CLK_SPEED * 2));
}


void Sensor::stepCMOS()
{
    CMOSToggle = !CMOSToggle; 

    digitalWriteFast(CMOS_CLK, CMOSToggle ? HIGH : LOW);
    //Serial.printf("%d\n", CMOSHalfCycles);
    //Start-time Done
    if (CMOSHalfCycles == CMOSTStartCycles)
    {
        Serial.println("CMOSSTARTDONE");
        //delay(500);
        digitalWriteFast(ST, LOW);
        //Begin the SPI stuff here because it needs to feed SCLK
        
        digitalWrite(SS, SELECTION_STATE);
        SPI.beginTransaction(spiSettings);

        //Setup the interupt so checking for the trig pin
        pinMode(EOS, INPUT_PULLUP);
        attachInterrupt(EOS, ADCReceive, FALLING);//MAKE SURE TO DETACH
        SPI.usingInterrupt(EOS);
    } 

    if (CMOSHalfCycles <= CMOSTStartCycles)
        CMOSHalfCycles++;

    /*static int testNum = 0;
    testNum = !testNum;
    //TESTING TESTING TESTING TESTING:     
    if (testNum == 1 && CMOSHalfCycles > START_CYCLE)
    {
        digitalWriteFast(33, LOW);
        ADCReceive();
        digitalWrite(33, HIGH);
    }*/
}


void Sensor::ADCReceive()
{
    //This accounts for the CMOS's integration time which is TRIG_OVER trig cycles after ST pulled low
    if (adcDataCount > TRIG_OVER)
    {
        int accumulates = 16;
        for (int i = 0; i < accumulates; i++)
        {
            digitalWriteFast(TRIG_OUT, HIGH);
            delayMicroseconds(1);
            digitalWriteFast(TRIG_OUT, LOW);
            if (i != accumulates - 1)
                delayMicroseconds(1);
        }
        //ADC outputs 14 bits so shift it left 2 so that its scaled correctly
        adcData[adcDataCount] = (SPI.transfer16(0) >> 2);
        digitalWriteFast(TRIG_OUT, HIGH);
        //Serial.println(adcData[adcDataCount]);

        //Check for final pixel. if end deinitialize sensor systems
        if (adcDataCount == PIXEL_COUNT + TRIG_OVER) // + TRIG_OVER because we are PIXEL_COUNT after the integration time
        {
            //Serial.println("CMOS DONE");
            SPI.endTransaction();
            detachInterrupt(EOS);
            digitalWrite(ST, LOW);
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


void Sensor::setStartCycles(int msec)
{
    if (msec < 100)
        msec = 100;
        
    CMOSTStartCycles = msec / (1000. / (CMOS_CLK_SPEED * 2));
    Serial.println(CMOSTStartCycles);
}