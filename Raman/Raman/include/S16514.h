#ifndef S16514_H
#define S16514_H

#include <stdint.h>
#include <Pin_Assignments.h>
#include <Arduino.h>

#define KSPS 500
#define INVALID_PIXELS 22
#define ST_CYCLES 7
#define INTEGRATION_CYCLES 48

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

        static bool s_cdcClockState;
        static bool s_adcClockState;

        IntervalTimer stepTimer;
};

#endif 