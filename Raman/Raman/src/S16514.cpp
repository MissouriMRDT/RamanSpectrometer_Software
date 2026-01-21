#include "S16514.h"

//Initializes the clock timer and any necessary values.
uint16_t* S16514::read()
{
    s_adcClockState = false;

    bool timerStarted = stepTimer.begin(stepRead, (1000000 / KSPS)/2);
}

void S16514::stepRead()
{
    s_adcClockState = !s_adcClockState;
    digitalWriteFast(ADC_CLK, s_adcClockState ? HIGH : LOW);

    s_adcSteps++;
    if (s_adcSteps % )
}