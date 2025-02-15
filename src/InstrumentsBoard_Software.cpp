#include "InstrumentsBoard_Software.h"
#include <Arduino.h>

void setup() {
  // Serial Debugger
  Serial.begin(115200);
  Serial.println("Instruments Setup");

  //miniSpec.init();
  RamanCCD.init(40000);
  
  pinMode(GREEN_LASER, OUTPUT);
  pinMode(SEL, OUTPUT);
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(FWD_LIM, INPUT_PULLDOWN);
  pinMode(RVS_LIM, INPUT_PULLDOWN);
  
  digitalWrite(GREEN_LASER, LOW);
  // TODO: remove SEL in Rev2
  digitalWrite(SEL, HIGH);

  InstrumentGantryMotor.init();

  forwardLimit.configInvert(true);
  reverseLimit.configInvert(true);
  InstrumentGantry.attachHardLimits(&reverseLimit, &forwardLimit);

  Serial.println("RoveComm Initializing...");
  RoveComm.begin(RC_INSTRUMENTSBOARD_IPADDRESS);
  Serial.println("Complete");
}


void loop() {

  RoveCommPacket packet;
  RoveComm.read(packet);

  switch (packet.dataId) {
    
    // Toggle LEDs
    case RC_INSTRUMENTSBOARD_ENABLELEDS_DATA_ID:
    {
      uint8_t data = ((uint8_t*) packet.data)[0];
      digitalWrite(GREEN_LASER, (data & 1<<0));
      // digitalWrite(WHITE_LED, (data & 1<<1));
      break;
    }
    
    // Request Raman
    case RC_INSTRUMENTSBOARD_REQUESTRAMANREADING_DATA_ID:
    {
      uint32_t data = ((uint32_t*) packet.data)[0];
      RamanCCD.setIntegrationTime(data);

      Serial.print("Raman: ");
      Serial.println(data);
      
      uint16_t pixels[2048];
      RamanCCD.read(pixels);

      RoveComm.write(RC_INSTRUMENTSBOARD_RAMANREADING_PART1_DATA_ID, 500, &pixels[0]);
      RoveComm.write(RC_INSTRUMENTSBOARD_RAMANREADING_PART2_DATA_ID, 500, &pixels[500]);
      RoveComm.write(RC_INSTRUMENTSBOARD_RAMANREADING_PART3_DATA_ID, 500, &pixels[1000]);
      RoveComm.write(RC_INSTRUMENTSBOARD_RAMANREADING_PART4_DATA_ID, 500, &pixels[1500]);
      RoveComm.write(RC_INSTRUMENTSBOARD_RAMANREADING_PART5_DATA_ID, 48,  &pixels[2000]);
      break;
    }
  }

  if (!digitalRead(SW1) && digitalRead(SW2))
  {
    InstrumentGantry.drive(900);
  }
  else if (digitalRead(SW1) && !digitalRead(SW2))
  {
    InstrumentGantry.drive(-900);
  }
  else
  {
    InstrumentGantry.drive(0);
  }
}
