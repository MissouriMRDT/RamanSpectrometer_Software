#include "Raman_Software.h"

#define BAUDRATE 0

#define ROG_PULSE_PERIOD 5000 //ns

void setup(){
    Serial.begin(BAUDRATE);
    Serial.println("setup");

    pinMode(CLK, OUTPUT);
    pinMode(ROG, OUTPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(RED, OUTPUT);
    pinMode(VOUT, INPUT);
}

void loop(){

    //float timestamp = ((float) millis()) / 1000.0;

/*
    read signal from rovecomm:
    set lights to whatever command is given
    watchdog setup to turn off lights if there is no communication???
    if rovecomm asks the raman spectrometer to get aquisitions, run the ccd, store the information, send the averaged signal back
    while the ccd is not aquiring data, set the signals to the idle values at the beginning of the pulse diagram

*/









    bool green = false;
    bool red = false;


    digitalWrite(GREEN, green? HIGH : LOW);
    digitalWrite(RED, red? HIGH : LOW);   
}