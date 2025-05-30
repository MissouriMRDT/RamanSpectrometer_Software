#include "InstrumentsBoard_Software.h"
#include <Arduino.h>

void setup() {
  // Serial Debugger
  Serial.begin(115200);
  //Serial.println("Instruments Setup");

  //miniSpec.init();
  RamanCCD.init(10000);

  pinMode(GREEN_LASER, OUTPUT);
  pinMode(SEL, OUTPUT);
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(LIMIT_SWITCH_1, INPUT_PULLDOWN);
  pinMode(LIMIT_SWITCH_2, INPUT_PULLDOWN);
  // disable just in case
  pinMode(34, INPUT_DISABLE);
  forwardLimit.configInvert(false);
  reverseLimit.configInvert(false);
  
  digitalWrite(GREEN_LASER, LOW);
  // TODO: remove SEL in Rev2
  digitalWrite(SEL, HIGH);

  InstrumentGantryMotor.init();

  //forwardLimit.configInvert(true);
  //reverseLimit.configInvert(true);
  InstrumentGantry.attachHardLimits(&reverseLimit, &forwardLimit);

  rollServo.attach(GIMBAL_PWM_A, 544, 2400);
  pitchServo.attach(GIMBAL_PWM_B);

  //Serial.println("RoveComm Initializing...");
  RoveComm.begin(RC_RAMANBOARD_IPADDRESS);
  //Serial.println("Complete");

  pinMode(LED_BUILTIN, OUTPUT);
}


void loop() {

  /*if (Serial.available()) {
    delay(10);
    String command = Serial.readString().trim();
    if (command == "L") {
      // turn on laser
      digitalWrite(FAN_OUT, HIGH);
    } else if (command == "l") {
      // turn off laser
      digitalWrite(FAN_OUT, LOW);
    } else if (command == "r") {
      targetRoll += 10;
      Serial.printf("Roll at %d \n", rollServo.read());
    } else if (command == "p") {
      targetPitch += 10;
      Serial.printf("Pitch at %d", pitchServo.read());
    } else if (command == "-r") {
      targetRoll -= 10;
      Serial.printf("Roll at %d \n", rollServo.read());
    } else if (command == "-p") {
      targetPitch -= 10;
      Serial.printf("Pitch at %d \n", pitchServo.read());
    } else {
      // set integration time and take raman reading
      int integrationTime = command.toInt();
      if (integrationTime >= 0) {
        RamanCCD.setIntegrationTime(integrationTime);
        uint16_t pixels[2048];
        memset(pixels, 0, sizeof(pixels));
        // digitalWrite(LED_BUILTIN, HIGH);
        RamanCCD.read(pixels);
        // digitalWrite(LED_BUILTIN, LOW);
        // Serial.println("Raman Data:");
        for (int i = 0; i < 2048; i++) {
          Serial.print(pixels[i]);
          Serial.print(", ");
        }
        Serial.println();
      }
    }
      
  }
    */
    // Serial.println(integrationTime);
  
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
      digitalWrite(FAN_OUT, packet.u8data[0]);
      break;
    }
    
    // Request Raman
    case RC_RAMANBOARD_REQUESTRAMANREADING_DATA_ID:
    {
      Serial.end();
      uint32_t data = ((uint32_t*) packet.data)[0];
      RamanCCD.setIntegrationTime(data);

      //Serial.print("Raman: ");
      //Serial.println(data);
      
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

    case RC_RAMANBOARD_RAMANGIMBALINCREMENT_DATA_ID:
    {
        int16_t* data = (int16_t*) packet.data;
        targetRoll = rollServo.read() + data[0];
        targetPitch = pitchServo.read() + data[1];
    }
  }

  rollServo.write(targetRoll);
  pitchServo.write(targetPitch);

  if (rollServo.read() > ROLL_UPPER_LIMIT)
  {
    targetRoll = ROLL_UPPER_LIMIT;
    rollServo.write(ROLL_UPPER_LIMIT);
  }
  else if (rollServo.read() < ROLL_LOWER_LIMIT)
  {
    targetRoll = ROLL_LOWER_LIMIT;
    rollServo.write(ROLL_LOWER_LIMIT);
  }
  
  if (pitchServo.read() > PITCH_UPPER_LIMIT)
  {
    targetPitch = PITCH_UPPER_LIMIT;
    pitchServo.write(PITCH_UPPER_LIMIT);
  }
  else if (pitchServo.read() < PITCH_LOWER_LIMIT)
  {
    targetPitch = PITCH_LOWER_LIMIT;
    pitchServo.write(PITCH_LOWER_LIMIT);
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
