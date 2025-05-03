#include "InstrumentsBoard_Software.h"
#include <Arduino.h>

uint16_t background[2048];
bool hasBackground = false;

void setup() {
  // Serial Debugger
  Serial.begin(115200);
  Serial.println("Instruments Setup");

  //miniSpec.init();
  RamanCCD.init(50'000);
  RamanCCD.setIntegrationTime(5000);

  pinMode(GREEN_LASER, OUTPUT);
  pinMode(SEL, OUTPUT);
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(GIMBAL_PWM_A, OUTPUT);
  pinMode(GIMBAL_PWM_B, OUTPUT);
  pinMode(LIMIT_SWITCH_1, INPUT_PULLDOWN);
  pinMode(LIMIT_SWITCH_2, INPUT_PULLDOWN);
  pinMode(34, INPUT_DISABLE); // TODO: remove
  forwardLimit.configInvert(false);
  reverseLimit.configInvert(false);
  
  digitalWrite(GREEN_LASER, LOW);
  // TODO: remove SEL in Rev2
  digitalWrite(SEL, HIGH);

  InstrumentGantryMotor.init();

  //forwardLimit.configInvert(true);
  //reverseLimit.configInvert(true);
  InstrumentGantry.attachHardLimits(&reverseLimit, &forwardLimit);

  Serial.println("RoveComm Initializing...");
  RoveComm.begin(RC_RAMANBOARD_IPADDRESS);
  Serial.println("Complete");

  pinMode(LED_BUILTIN, OUTPUT);
}

void takeBackgroundReading() {
  memset(background, 0, sizeof(background));
  RamanCCD.read(background);
  for (int i = 0; i < 2048; i++) {
    Serial.print(background[i]);
    Serial.print(", ");
  }
  Serial.println();

  hasBackground = true;
}

void takeRamanReading() {
  // Serial.println("Taking Raman Reading...");
  uint16_t pixels[2048];
  memset(pixels, 0, sizeof(pixels));
  // digitalWrite(LED_BUILTIN, HIGH);
  RamanCCD.read(pixels);
  // digitalWrite(LED_BUILTIN, LOW);
  // Serial.println("Raman Data:");
  for (int i = 0; i < 2048; i++) {
    if (hasBackground) {
      Serial.print(max((int)background[i] - (int)pixels[i], 0));
    } else {
      Serial.print(pixels[i]);
    }
    Serial.print(", ");
  }
  Serial.println();
}

void loop() {

  if (Serial.available()) {
    delay(100);
    int integrationTime = Serial.readString().trim().toInt();
    if (integrationTime >= 0) {
      RamanCCD.setIntegrationTime(integrationTime);
      takeRamanReading();
    }
    // Serial.println(integrationTime);
  }

  if (!digitalRead(SW1) && digitalRead(SW2))
  {
    // InstrumentGantry.drive(-1000);
    // feedWatchdog();
    takeRamanReading();
    delay(1000);
  }
  else if (digitalRead(SW1) && !digitalRead(SW2))
  {
    // InstrumentGantry.drive(1000);
    // feedWatchdog();
    takeBackgroundReading();
    delay(1000);
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
      RamanCCD.setIntegrationTime(data);

      Serial.print("Raman: ");
      Serial.println(data);
      
      uint16_t pixels[2048];
      digitalWrite(LED_BUILTIN, HIGH);
      RamanCCD.read(pixels);
      digitalWrite(LED_BUILTIN, LOW);

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
