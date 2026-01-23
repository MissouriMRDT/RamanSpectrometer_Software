#ifndef S16514_H
#define S16514_H

#include <stdint.h>
#include <Pin_Assignments.h>
#include <Arduino.h>
#include <math.h> 

#define KSPS 500
#define INVALID_PIXELS 22
#define ST_CYCLES 7
#define INTEGRATION_CYCLES 48
#define VALID_PIXELS 2008

#define ADC_TO_SENSOR_CYCLES 28
#define ADC_DATA_CYCLES 14

class S16514
{
    public:
        S16514();
        ~S16514();

        uint16_t* read();
    private:
        static void stepRead(void);

        static int s_cmosSteps;
        static int s_adcSteps;
        static int s_currentPixel;

        static bool s_cmosClockState;
        static bool s_adcClockState;
        static bool s_read;

        static uint16_t* s_lightData;

        static void stepCMOS();
        static void stepADC();

        IntervalTimer stepTimer;
};

#endif 