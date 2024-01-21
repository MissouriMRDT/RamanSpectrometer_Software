#include "Raman_Software.h"
#include <cmath>

#define BAUDRATE 0

#define CLK_FREQUENCY 1000000.0 //Hz
#define CLK_TIMEPERIOD (1.0 / CLK_FREQUENCY) //seconds

#define ROG_t7 5000 //ns
#define CLK_t5 3000
#define CLK_t9 3000

#define PIXEL_COUNT 2048
#define CLK_REPETITIONS 2088
#define TEENSY_TIMETOHIGH 0//TODO measure
#define TEENSY_TIMETOLOW 0 //TODO measure

#define SH_MODE true//TODO make both options

double startResetTimestamp = 0;
double startAquiringTimestamp = 0;


int pixelArray[PIXEL_COUNT];

enum ClockingState
{
    idling, resetting, aquiring
};

ClockingState clockingState = idling;

double getTimestamp(){
    return ((float) millis()) / 1000.0;
}

void setup(){
    Serial.begin(BAUDRATE);
    Serial.println("setup");

    pinMode(CLK, OUTPUT);
    pinMode(ROG, OUTPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(RED, OUTPUT);
    pinMode(VOUT, INPUT);

    Serial.println(CLK_TIMEPERIOD);
}

void loop(){


/*
    read signal from rovecomm:
    set lights to whatever command is given
    watchdog setup to turn off lights if there is no communication???
    if rovecomm asks the raman spectrometer to get aquisitions, run the ccd, store the information, send the averaged signal back
    while the ccd is not aquiring data, set the signals to the idle values at the beginning of the pulse diagram

*/


    float timestamp = getTimestamp();
    //variables to read from rovecomm
    bool green = false;
    bool red = false;
    bool aquireData = true; //TODO maybe have an integer representing the amount of aquisitions to take, then run the ccd for that many aquisitions, then average them, and set the ccd to idle when finished




    digitalWrite(GREEN, green? HIGH : LOW);
    digitalWrite(RED, red? HIGH : LOW);









    if(aquireData){
        if(clockingState == idling){ //if we are idling and we want to aquire data, we reset
            startResetTimestamp = getTimestamp();
            clockingState = resetting;
        }

        if(clockingState == resetting){ // the resetting state is the dip in the ROG before the normal clocking signal
            //logic for the reset sequence at the beginning
            if(timestamp < startResetTimestamp + CLK_t5){
                digitalWrite(CLK, HIGH);
                digitalWrite(ROG, HIGH);
            } else if(timestamp < startResetTimestamp + CLK_t5 + ROG_t7){
                digitalWrite(CLK, HIGH);
                digitalWrite(ROG, LOW);
            } else if(timestamp < startResetTimestamp + CLK_t5 + ROG_t7 + CLK_t9){
                digitalWrite(CLK, HIGH);
                digitalWrite(ROG, HIGH);
            } else{
                startAquiringTimestamp = getTimestamp();
                clockingState = aquiring;
            }
        }
        



        if(clockingState == aquiring){ 
            //logic for CLK
                // (timestamp-startAquiringTimestamp)/CLK_TIMEPERIOD == number of cycles of the clock
                // floor((timestamp-startAquiringTimestamp)/CLK_TIMEPERIOD) == number of cycles fully complete(rounded down)
                // floor((timestamp-startAquiringTimestamp)/CLK_TIMEPERIOD) * CLK_TIMEPERIOD == the start time of the latest cycle
                // timestamp - floor((timestamp-startAquiringTimestamp)/CLK_TIMEPERIOD) * CLK_TIMEPERIOD == how far we are into the current cycle
                // timestamp - floor((timestamp-startAquiringTimestamp)/CLK_TIMEPERIOD) * CLK_TIMEPERIOD < (CLK_TIMEPERIOD/2) == true if we are in the first half of the cycle
            if( timestamp - floor((timestamp-startAquiringTimestamp)/CLK_TIMEPERIOD) * CLK_TIMEPERIOD < (CLK_TIMEPERIOD/2)){ //if we are at the first half of the period, set LOW
                digitalWrite(CLK, LOW);
            } else {//in the second half, set HIGH
                digitalWrite(CLK, HIGH);
            }

            //logic for ROG
            digitalWrite(ROG, HIGH);
        }

    } else{// if the ccd is idling

        digitalWrite(CLK, LOW);
        digitalWrite(ROG, LOW);
        delay(1000); //TODO REMOVE THIS, THIS IS TO ADD A DELAY IN BETWEEN RUNS FOR TESTING
    }



}