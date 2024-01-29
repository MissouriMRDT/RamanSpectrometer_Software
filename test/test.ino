#include <iostream>


#define CLK_FREQUENCY 1000000.0 //Hz
#define CLK_TIMEPERIOD (1.0 / CLK_FREQUENCY) //seconds
#define HALF_CLK_TIMEPERIOD (CLK_TIMEPERIOD/2.0)

double clockCyclesRemaining = 0;
void setup(){

  Serial.begin(115200);
    Serial.print("starting");
    pinMode(25, OUTPUT);
    
    analogWriteFrequency(25, 1000000);
    analogWrite(25, 128);
}

void loop(){
  clockCyclesRemaining = 10;

  
    analogWrite(25, 128);
    delay(1000);
    
    analogWrite(25, 0);
    delay(1000);
    



//   while(clockCyclesRemaining > 0){
//     digitalWrite(25, HIGH);
//     delayNanoseconds(500);
//     digitalWrite(25, LOW); 
//     delayNanoseconds(500);
//     clockCyclesRemaining--;


//   }  

// delay(50);


}