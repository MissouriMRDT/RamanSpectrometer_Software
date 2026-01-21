#include "Raman_Software.h"


void setup()
{
  pinMode(FAN_OUT, OUTPUT);
  pinMode(LASER_OUT, OUTPUT);

  //Initialize RoveComm to the core board 
  roveComm.begin(RC_RAMANBOARD_IPADDRESS);
}

void loop() {
  //Check for RoveComm Packets:
  roveComm.read(packet); 
  
  //process RoveComm Packets:
  switch (packet.dataId)
  { 
  //Laser Toggle
  case RC_RAMANBOARD_LASER_DATA_ID:
    digitalWrite(FAN_OUT, packet.i8data[0]);
    digitalWrite(LASER_OUT, packet.i8data[0]);
    break;
  case RC_RAMANBOARD_REQUESTRAMANREADING_DATA_ID:
    uint16_t* pixels;
    pixels = linearSensor.read();    

    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART1_DATA_ID, (VALID_PIXELS/4), &pixels[0]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART2_DATA_ID, (VALID_PIXELS/4), &pixels[(VALID_PIXELS/4)]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART3_DATA_ID, (VALID_PIXELS/4), &pixels[(VALID_PIXELS/4) * 2]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART4_DATA_ID, (VALID_PIXELS/4), &pixels[(VALID_PIXELS/4) * 3]);
    
    break;
  }
}
