#include "Raman_Software.h"

#define BAUDRATE 0

#define CLK_FREQUENCY 2000000.0 //Hz
#define CLK_DURATION (1/CLK_FREQUENCY) //seconds

#define ROG_PULSE_PERIOD 5000 //ns

bool startAquisitionTimestamp = 0;

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

    Serial.println(CLK_DURATION);
}

void loop(){

    //float timestamp = getTimestamp();

/*
    read signal from rovecomm:
    set lights to whatever command is given
    watchdog setup to turn off lights if there is no communication???
    if rovecomm asks the raman spectrometer to get aquisitions, run the ccd, store the information, send the averaged signal back
    while the ccd is not aquiring data, set the signals to the idle values at the beginning of the pulse diagram

*/


    //variables to read from rovecomm
    bool green = false;
    bool red = false;
    bool aquireData = true;




    digitalWrite(GREEN, green? HIGH : LOW);
    digitalWrite(RED, red? HIGH : LOW);









    if(aquireData){
        if(beginning of clock sequence){
            startAquisitionTimestamp = getTimestamp();
        }

        //logic for CLK


        //logic for ROG


        //logic for 

    } else{// if the ccd is idling
        digitalWrite(CLK, HIGH);
        digitalWrite(ROG, LOW);
        digitalWrite()
    }



}