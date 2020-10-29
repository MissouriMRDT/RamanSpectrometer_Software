/*
 * Macro Definitions
 */
#include <Energia.h>
#include <Ethernet.h>
#include "RoveComm.h"

#define SPEC_TRG         PK_0
#define SPEC_ST          PK_1
#define SPEC_CLK         PK_2
#define SPEC_VIDEO       PK_3

#define SPEC_CHANNELS    288 // New Spec Channel
#define SPECTROMETER_TELEM_RATE 5000
uint16_t data[SPEC_CHANNELS];

//rovecomm and packet instances
RoveCommEthernet RoveComm;
rovecomm_packet packet; 

//timekeeping variables
uint32_t last_update_time;

//declare the Ethernet Server in the top level sketch with the requisite port ID any time you want to use RoveComm
EthernetServer TCPServer(RC_ROVECOMM_ETHERNET_SRA_SENSORBOARD_PORT);

void setup(){

  //Set desired pins to OUTPUT
  pinMode(SPEC_CLK, OUTPUT);
  pinMode(SPEC_ST, OUTPUT);

  digitalWrite(SPEC_CLK, HIGH); // Set SPEC_CLK High
  digitalWrite(SPEC_ST, LOW); // Set SPEC_ST Low

  Serial.begin(115200); // Baud Rate set to 115200  
  RoveComm.begin(RC_SRASENSORSBOARD_FOURTHOCTET, &TCPServer);
  
  delay(100);
  last_update_time = millis();


}

/*
 * This functions reads spectrometer data from SPEC_VIDEO
 * Look at the Timing Chart in the Datasheet for more info
 */
void readSpectrometer(){

  int delayTime = 1; // delay time

  // Start clock cycle and set start pulse to signal start
  digitalWrite(SPEC_CLK, LOW);
  delayMicroseconds(delayTime);
  digitalWrite(SPEC_CLK, HIGH);
  delayMicroseconds(delayTime);
  digitalWrite(SPEC_CLK, LOW);
  digitalWrite(SPEC_ST, HIGH);
  delayMicroseconds(delayTime);
  
  //Sample for a period of time
  for(int i = 0; i < 15; i++){

      digitalWrite(SPEC_CLK, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(SPEC_CLK, LOW);
      delayMicroseconds(delayTime); 
 
  }

  //Set SPEC_ST to low
  digitalWrite(SPEC_ST, LOW);

  //Sample for a period of time
  for(int i = 0; i < 85; i++){

      digitalWrite(SPEC_CLK, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(SPEC_CLK, LOW);
      delayMicroseconds(delayTime); 
      
  }

  //One more clock pulse before the actual read
  digitalWrite(SPEC_CLK, HIGH);
  delayMicroseconds(delayTime);
  digitalWrite(SPEC_CLK, LOW);
  delayMicroseconds(delayTime);
 
  //Read from SPEC_VIDEO
  for(int i = 0; i < SPEC_CHANNELS; i++){

      data[i] = analogRead(SPEC_VIDEO);
      
      digitalWrite(SPEC_CLK, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(SPEC_CLK, LOW);
      delayMicroseconds(delayTime);
        
  }
  
  //Set SPEC_ST to high
  digitalWrite(SPEC_ST, HIGH);

  //Sample for a small amount of time
  for(int i = 0; i < 7; i++){
    
      digitalWrite(SPEC_CLK, HIGH);
      delayMicroseconds(delayTime);
      digitalWrite(SPEC_CLK, LOW);
      delayMicroseconds(delayTime);
    
  }
  
  digitalWrite(SPEC_CLK, HIGH);
  
  delayMicroseconds(delayTime);
  
}

/*
 * The function below prints out data to the terminal or 
 * processing plot
 */
void sendData(){
  uint16_t first_half[144];
  uint16_t second_half[144];

  for(int i = 0; i < 144; i++)
  {
    first_half[i] = data[i];
    second_half[i] = data[(288-1)-i];
  }
  
  if(millis()-last_update_time >= SPECTROMETER_TELEM_RATE)
  {
      RoveComm.writeReliable(9101, 144, first_half);
      RoveComm.writeReliable(9101, 144, second_half);
      last_update_time = millis();
  }
}

void loop(){

   packet = RoveComm.read();

  //Write some sample data back every 100 milliseconds, it is important that any
  //telemetry is NOT rate limited (using delays) as this will prevent
  //packets from arriving in a timely manner 
  
  readSpectrometer();
  sendData();
  delay(10);  
}
