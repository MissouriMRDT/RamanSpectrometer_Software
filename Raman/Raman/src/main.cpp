#include "Raman_Software.h"


void setup()
{
  //Setup Outputs
  pinMode(FAN_OUT, OUTPUT);
  digitalWrite(FAN_OUT, LOW);
  pinMode(LASER_OUT, OUTPUT);
  digitalWrite(LASER_OUT, LOW);

  //Inputs:
  pinMode(LIMIT_SW1, INPUT_PULLDOWN);
  pinMode(LIMIT_SW2, INPUT_PULLDOWN);
  pinMode(CAN_SW1, INPUT_PULLDOWN);
  pinMode(CAN_SW2, INPUT_PULLDOWN);

  //Initialize RoveComm to the core board 
  roveComm.begin(RC_RAMANBOARD_IPADDRESS);

  //Initialize telementary data timer
  telemetryCounter = millis();
}

void loop() {
  //Check Limit Switches:
  //if the bottom switch activated gantry can only move up, 
  //if upper switch activated gantry can only move down, 
  //if neither the gantry is able to move freely.
  if (digitalRead(LIMIT_SW1) && instrumentGantrySpeed > 0 && 
    digitalRead(LIMIT_SW2) && instrumentGantrySpeed < 0)
  {
   //drive 0 
  }
  else
  {
    //drive to instrument Gantry Speed 
  }
  
  //Check CAN Buttons
  if (digitalRead(CAN_SW1))
  {
    //TODO Something with CAN
  }
  if (digitalRead(CAN_SW2))
  {
    //TODO Something with CAN
  }

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
  //Read Raman Data
  case RC_RAMANBOARD_REQUESTRAMANREADING_DATA_ID:
    uint16_t* pixels;
    pixels = linearSensor.read();    

    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART1_DATA_ID, (VALID_PIXELS/4), &pixels[0]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART2_DATA_ID, (VALID_PIXELS/4), &pixels[(VALID_PIXELS/4)]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART3_DATA_ID, (VALID_PIXELS/4), &pixels[(VALID_PIXELS/4) * 2]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART4_DATA_ID, (VALID_PIXELS/4), &pixels[(VALID_PIXELS/4) * 3]);
    break;

  case RC_RAMANBOARD_INSTRUMENTSAXIS_DATA_ID:
    instrumentGantrySpeed = packet.i16data[0] >= 0 ? packet.i16data[0] / 32767. : packet.i16data[0] / 32768.;

    break;
  case RC_RAMANBOARD_WATCHDOGOVERRIDE_DATA_ID:
    watchdogOverride = packet.i8data[0];

    break;
  }

  if (millis() - telemetryCounter)
  {
    roveComm.write(RC_RAMANBOARD_POSITION_DATA_ID, );

    telemetryCounter = millis();
  }
}

//Watchdog Stuff
void estop() {
    if (!watchdogOverride) {
      //TODO drive the gantry to 0
    }
}


void feedWatchdog() {
    Watchdog.begin(estop, WATCHDOG_TIMEOUT);
}