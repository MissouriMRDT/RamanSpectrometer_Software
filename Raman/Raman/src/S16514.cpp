#include "S16514.h"

//Initializes the clock timer and any necessary values.
uint16_t* S16514::read()
{   
    pinMode(CMOS_CLK, OUTPUT);
    pinMode(ADC_CLK, OUTPUT);
    pinMode(ST, OUTPUT);
    pinMode(LIGHT_IN, INPUT);

    s_adcClockState = false;
    s_cmosClockState = false;
    s_adcSteps = 0;
    s_cmosSteps = 0;
    s_currentPixel = 0;

    s_read = false;

    s_lightData = new uint16_t[VALID_PIXELS];
    //TODO: error checking
    bool timerStarted = stepTimer.begin(stepRead, (1000000 / KSPS)/2);

    delayMicroseconds((INVALID_PIXELS + ST_CYCLES + INTEGRATION_CYCLES + VALID_PIXELS) * (1000 / KSPS));
    return s_lightData;
}

void S16514::stepRead()
{
    s_adcClockState = !s_adcClockState;

    //Step adc only on rising edge, and step cmos only on rising edge every ADC_TO_SENSOR_CYCLES adc steps.
    if (s_adcClockState)
    {
        if (s_adcSteps % (ADC_TO_SENSOR_CYCLES / 2) == 0)
        {
            s_cmosClockState = !s_cmosClockState;
            
            if (s_cmosClockState)
            {
                s_cmosSteps++;
                stepCMOS();
            }
            //write CMOS clock states to the teensy.
            digitalWriteFast(CMOS_CLK, s_cmosClockState ? HIGH : LOW);
        }

        s_adcSteps++;
        stepADC();
    }
    
    //write ADC clock states to the teensy.
    digitalWriteFast(ADC_CLK, s_adcClockState ? HIGH : LOW);
    stepADC();
}


void S16514::stepCMOS()
{
    //Triggers the start pin beginning the integration cycles
    switch (s_cmosClockState)
    {
        case 1:
            digitalWriteFast(ST, HIGH);
            break;
        case ST_CYCLES:
            digitalWriteFast(ST, LOW);
            break;
        case ST_CYCLES + INTEGRATION_CYCLES + INVALID_PIXELS:
            s_read = true;
            break;
    }
}


void S16514::stepADC()
{
    //only read data if in valid pixels and in ADC output cycle
    if (s_read && (s_adcSteps % ADC_TO_SENSOR_CYCLES) > (ADC_TO_SENSOR_CYCLES - ADC_DATA_CYCLES))
    {
        //Set the current bit of the inputing data
        s_lightData[s_currentPixel] |= (digitalRead(LIGHT_IN) ? 1U : 0U << ((s_adcSteps % ADC_TO_SENSOR_CYCLES) - (ADC_TO_SENSOR_CYCLES - ADC_DATA_CYCLES)));

        s_currentPixel++;
    }
}