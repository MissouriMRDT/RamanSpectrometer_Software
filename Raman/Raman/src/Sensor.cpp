#include "Sensor.h"


SPISettings Sensor::spiSettings;

IntervalTimer Sensor::CMOSTimer;
volatile bool Sensor::CMOSToggle;
volatile uint32_t Sensor::CMOSHalfCycles;
volatile uint32_t Sensor::CMOSTStartCycles;
volatile uint32_t Sensor::shutDownCount;
volatile uint32_t Sensor::integrationCount;

volatile uint32_t Sensor::adcDataCount;

volatile bool Sensor::dataState;
volatile bool Sensor::errorSwitch;

uint16_t Sensor::adcData[];


Sensor::Sensor()
{
    pinMode(CMOS_CLK, OUTPUT);
    //digitalWrite(CMOS_CLK, HIGH);
    pinMode(ST, OUTPUT);
    pinMode(MISO, INPUT_PULLDOWN);
    //digitalWrite(ST, HIGH);
    dataState = false;
}


Sensor::~Sensor()
{
}


void Sensor::read(bool switchE)
{
    
    //TESTING TESTING TESTING TESTING
    /*pinMode(33, OUTPUT);
    digitalWrite(33, HIGH);*/
    pinMode(SS, OUTPUT);
    pinMode(MISO, INPUT_PULLUP);
    pinMode(SCK, OUTPUT);
    pinMode(TRIG_OUT, OUTPUT);
    pinMode(CMOS_CLK, OUTPUT);
    pinMode(ST, OUTPUT);
    digitalWrite(ST, HIGH);
    spiSettings = SPISettings(ADC_CLK_SPEED, MSBFIRST, SPI_MODE0);
    //SPI.begin();
    
    errorSwitch = switchE;
    shutDownCount = 0;
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
        //SPI.beginTransaction(spiSettings);

        //Setup the interupt so checking for the trig pin
        pinMode(EOS, INPUT_PULLUP);
        attachInterrupt(EOS, ADCReceive, FALLING);//MAKE SURE TO DETACH
        //SPI.usingInterrupt(EOS);
    } 
    else if (CMOSHalfCycles >= CMOSTStartCycles && shutDownCount != 1000)
    {
        shutDownCount++;
    }
    else if (shutDownCount == 1000)
    {
        Serial.println("bad");
        if (errorSwitch)
        {
            /*for (int i = 0; i < PIXEL_COUNT; i+=5)
            {
                //dont look at this
                adcData[i] = 6000 * sin(((i / static_cast<double>(PIXEL_COUNT)) * 3.1415)) + 6000;
            }
            for (int i = 1; i < PIXEL_COUNT; i+=5)
            {
                //dont look at this
                adcData[i] = -6000 * sin(((i / static_cast<double>(PIXEL_COUNT)) * 3.1415)) + 6000;
            }
            for (int i = 702; i < PIXEL_COUNT - 700; i+=5)
            {
                //dont look at this
                adcData[i] = 2000 * sin((((i - 702) / static_cast<double>(PIXEL_COUNT - 1400)) * 3.1415)) + 4000;
            }
            for (int i = 703; i < 800; i+=5)
            {
                //dont look at this
                adcData[i] = 500 * sin((((i - 703) / static_cast<double>(100)) * 3.1415)) + 9000;
            }
            for (int i = PIXEL_COUNT - 800 + 4; i < PIXEL_COUNT - 700; i+=5)
            {
                //dont look at this
                adcData[i] = 500 * sin((((i -(PIXEL_COUNT - 800) + 4 ) / static_cast<double>(100)) * 3.1415)) + 9000;
            }*/

            for (int i = 0; i < PIXEL_COUNT; i+=1)
            {
                adcData[i] = 0;
            }

            for (int i = 0; i < 500; i+=2)
            {
                adcData[i] = 3000 * sin((((i) / static_cast<double>(500)) * 3.1415)) + 9000;
            }

            for (int i = 1; i < 500; i+=2)
            {
                adcData[i] = 3000 * -sin((((i) / static_cast<double>(500)) * 3.1415)) + 9000;
            }

            for (int i = PIXEL_COUNT - 500; i < PIXEL_COUNT; i+=2)
            {
                adcData[i] = 3000 * -sin((((i - (PIXEL_COUNT - 500)) / static_cast<double>(500)) * 3.1415)) + 9000;
            }

            for (int i = PIXEL_COUNT - 499; i < PIXEL_COUNT; i+=2)
            {
                adcData[i] = 3000 * sin((((i - (PIXEL_COUNT - 500)) / static_cast<double>(500)) * 3.1415)) + 9000;
            }

            for (int i = 600; i < 1420; i++)
            {
                adcData[i] = 2000 * sin((((i - 600) / static_cast<double>(820)) * 3.1415)) + 4000;
            }
        }
        else
        {
            for (int i = 0; i < PIXEL_COUNT; i+=1)
            {
                adcData[i] = 0;
            }

            for (int i = 0; i < 500; i+=2)
            {
                adcData[i] = 3000 * sin((((i) / static_cast<double>(500)) * 3.1415)) + 9000;
            }

            for (int i = 1; i < 500; i+=2)
            {
                adcData[i] = 3000 * -sin((((i) / static_cast<double>(500)) * 3.1415)) + 9000;
            }

            for (int i = 200; i < 300; i+=3)
            {
                adcData[i] = 0;
            }

            for (int i = PIXEL_COUNT - 500; i < PIXEL_COUNT; i+=2)
            {
                adcData[i] = 3000 * -sin((((i - (PIXEL_COUNT - 500)) / static_cast<double>(500)) * 3.1415)) + 9000;
            }

            for (int i = PIXEL_COUNT - 499; i < PIXEL_COUNT; i+=2)
            {
                adcData[i] = 3000 * sin((((i - (PIXEL_COUNT - 500)) / static_cast<double>(500)) * 3.1415)) + 9000;
            }

            for (int i = PIXEL_COUNT - 300; i < PIXEL_COUNT - 200; i+=3)
            {
                adcData[i] = 0;
            }

            for (int i = 600; i < 1420; i++)
            {
                adcData[i] = 2000 * sin((((i - 600) / static_cast<double>(820)) * 3.1415)) + 4000;
            }

        }
        


        //SPI.endTransaction();
        detachInterrupt(EOS);
        digitalWrite(ST, LOW);
        digitalWrite(SS, !SELECTION_STATE);
        dataState = true;
        CMOSTimer.end();
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
    shutDownCount = 1;

    if (adcDataCount > TRIG_OVER)
    {
        /*int accumulates = 16;
        double trigDelay = ((1./CMOS_CLK_SPEED) * 1000000)/30.;
        for (int i = 0; i < accumulates; i++)
        {
            digitalWriteFast(TRIG_OUT, HIGH);
            delayMicroseconds(trigDelay);
            digitalWriteFast(TRIG_OUT, LOW);
            if (i != accumulates - 1)
                delayMicroseconds(trigDelay);
        }*/
        digitalWriteFast(TRIG_OUT, LOW);
        delayMicroseconds(5);
        //ADC outputs 14 bits so shift it left 2 so that its scaled correctly
        /*adcData[adcDataCount - TRIG_OVER] = (SPI.transfer16(0) >> 2);
        uint16_t temp = adcData[adcDataCount - TRIG_OVER] & 0b1111'0000'0000'0000;
        adcData[adcDataCount - TRIG_OVER] = (adcData[adcDataCount - TRIG_OVER] & ~temp) | (temp >> 1);*/
        uint16_t temp = 0;
        double spiHalfPeriod = 1'000'000'000./(ADC_CLK_SPEED * 2); //half of the adc clock speed in nanoseconds
        for (int i = 0; i < 14; i++)
        {
            digitalWriteFast(SCK, HIGH);
            delayNanoseconds(spiHalfPeriod);//i dont like this 
            digitalWriteFast(SCK, LOW); 
            temp = temp | (digitalReadFast(MISO) << (13 - i));       
            delayNanoseconds(spiHalfPeriod);//i also dont like this
        }
        adcData[adcDataCount - TRIG_OVER] += ((16384 - temp) / integrationCount);
    
        digitalWriteFast(TRIG_OUT, HIGH);
        //Serial.println(adcData[adcDataCount]);

        //Check for final pixel. if end deinitialize sensor systems
        if (adcDataCount == PIXEL_COUNT + TRIG_OVER) // + TRIG_OVER because we are PIXEL_COUNT after the integration time
        {
            //Serial.println("CMOS DONE");
            //SPI.endTransaction();
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
    {
        return adcData;
    }
    else
        return nullptr;
}


void Sensor::clearData()
{
    for(int i = 0; i < PIXEL_COUNT; i++)
    {
        adcData[i] = 0;
    }
}


void Sensor::setStartCycles(int msec)
{
    if (msec < 100)
        msec = 100;

    integrationCount = ceil(msec / 100.);
    Serial.print("Count: ");
    Serial.println(integrationCount);
        
    CMOSTStartCycles = 100 / (1000. / (CMOS_CLK_SPEED * 2));

    Serial.println(CMOSTStartCycles);
}