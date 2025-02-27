#include "InstrumentsBoard_Software.h"
#include <Arduino.h>

void setup() {
  // Serial Debugger
  Serial.begin(115200);
  Serial.println("Instruments Setup");

  //miniSpec.init();
  RamanCCD.init(40000);
  RamanCCD.s_readPin = CCD_CLK_OUT;
  
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
  RoveComm.begin(RC_RAMANBOARD_IPADDRESS);
  Serial.println("Complete");
}


void loop() {
  
  if (!digitalRead(SW1) && digitalRead(SW2))
  {
    InstrumentGantry.drive(1000);
    feedWatchdog();
  }
  else if (digitalRead(SW1) && !digitalRead(SW2))
  {
    InstrumentGantry.drive(-1000);
    feedWatchdog();
  } else {
    InstrumentGantry.drive(targetSpeed);
  }

  RoveCommPacket packet;
  RoveComm.read(packet);

  switch (packet.dataId) {
    
    // Toggle LEDs
    case RC_RAMANBOARD_LASER_DATA_ID:
    {
      digitalWrite(GREEN_LASER, packet.u8data[0]);
      break;
    }
    
    // Request Raman
    case RC_RAMANBOARD_REQUESTRAMANREADING_DATA_ID:
    {
      uint32_t data = ((uint32_t*) packet.data)[0];
      //RamanCCD.setIntegrationTime(data);

      Serial.print("Raman: ");
      Serial.println(data);
      
      uint16_t pixels[2048];
      //RamanCCD.read(pixels);

      for (int i = 0; i < 2048; i+=2)
      {
        int noise = rand() % 10;
        uint16_t tmp = abs(tryptophan_data[i/2]) * 1023.0 + (noise > 5 ? noise/2 : -noise/2);
        pixels[i] = tmp;
        pixels[i+1] = tmp;
      }

      RoveComm.write(RC_RAMANBOARD_RAMANREADING_PART1_DATA_ID, 512, &pixels[0]);
      RoveComm.write(RC_RAMANBOARD_RAMANREADING_PART2_DATA_ID, 512, &pixels[512]);
      RoveComm.write(RC_RAMANBOARD_RAMANREADING_PART3_DATA_ID, 512, &pixels[1024]);
      RoveComm.write(RC_RAMANBOARD_RAMANREADING_PART4_DATA_ID, 512, &pixels[1536]);
      break;
    }

    case RC_RAMANBOARD_INSTRUMENTSAXIS_OPENLOOP_DATA_ID:
    {
      targetSpeed = packet.i16data[0];
      feedWatchdog();
      break;
    }

    case RC_RAMANBOARD_WATCHDOGOVERRIDE_DATA_ID:
    {
      watchdogOverride = packet.u8data[0];
      break;
    }

    case RC_RAMANBOARD_LIMITSWITCHOVERRIDE_DATA_ID:
    {
      InstrumentGantry.overrideForwardHardLimit(packet.u8data[0] & (1 << 0));
      InstrumentGantry.overrideReverseHardLimit(packet.u8data[0] & (1 << 1));
    }
  }
}

void estop() {
    watchdogStatus = 1;
    if (!watchdogOverride) {
        InstrumentGantry.drive(0);
    }
}


void feedWatchdog() {
    watchdogStatus = 0;
    Watchdog.begin(estop, WATCHDOG_TIMEOUT);
}
