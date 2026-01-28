#include "Raman_Software.h"


void setup()
{
  
  Serial.begin(115200);

  //while(!Serial);

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

  //Initialize TOF
  Wire.begin();
  
  Wire.setSCL(TOF_SCL);
  Wire.setSDA(TOF_SDA);

  auto tofAddress = identifyTofAddress();
  delay(500);

  tofSensor->begin();
  tofSensor->VL53L4CX_Off();
  Serial.println(tofSensor->InitSensor(tofAddress));
  Serial.print("notStuck1");
  tofSensor->VL53L4CX_ClearInterruptAndStartMeasurement();
  Serial.println("notStuck");
}

void loop() {
  while (true)
  {
    VL53L4CX_MultiRangingData_t multiRangingData;
    uint8_t objectsFound = 0;
    float tofDis = 1000000;

    tofSensor->VL53L4CX_GetMultiRangingData(&multiRangingData);
    objectsFound = multiRangingData.NumberOfObjectsFound;

    for (int i = 0; i < objectsFound; i++)
    {
      float tempDis = multiRangingData.RangeData[i].RangeMilliMeter;
      if (tempDis < tofDis)
        tofDis = tempDis;
    }

    tofSensor->VL53L4CX_ClearInterruptAndStartMeasurement();

    Serial.printf("data: %d\n", multiRangingData.RangeData[0].RangeMilliMeter);

    delay(200);
  }
  delay(1000);
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
    /*uint16_t* pixels;
    //pixels = linearSensor.read();    

    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART1_DATA_ID, (VALID_PIXELS/4), &pixels[0]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART2_DATA_ID, (VALID_PIXELS/4), &pixels[(VALID_PIXELS/4)]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART3_DATA_ID, (VALID_PIXELS/4), &pixels[(VALID_PIXELS/4) * 2]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART4_DATA_ID, (VALID_PIXELS/4), &pixels[(VALID_PIXELS/4) * 3]);*/
    break;

  case RC_RAMANBOARD_INSTRUMENTSAXIS_DATA_ID:
    instrumentGantrySpeed = floor(packet.i16data[0] >= 0 ? packet.i16data[0] / 327.67 : packet.i16data[0] / 327.68);

    break;
  case RC_RAMANBOARD_WATCHDOGOVERRIDE_DATA_ID:
    watchdogOverride = packet.i8data[0];

    break;
  }

  //Gantry Button Inputs
  if (!digitalRead(CAN_SW1) && digitalRead(CAN_SW2))
    smoco.openLoopDrive(50);
  else if (digitalRead(CAN_SW1) && !digitalRead(CAN_SW2))
    smoco.openLoopDrive(-50);
  else 
    smoco.openLoopDrive(instrumentGantrySpeed);

  Serial.println("tofDi1s");

  if (millis() - telemetryCounter > 10)
  {
    //Tof Telementary Data Retrieval
    VL53L4CX_MultiRangingData_t multiRangingData;
    uint8_t objectsFound = 0;
    float tofDis = FLT_MAX;

    tofSensor->VL53L4CX_GetMultiRangingData(&multiRangingData);
    objectsFound = multiRangingData.NumberOfObjectsFound;

    for (int i = 0; i < objectsFound; i++)
    {
      float tempDis = multiRangingData.RangeData[i].RangeMilliMeter;
      if (tempDis < tofDis)
        tofDis = tempDis;
    }

    tofSensor->VL53L4CX_ClearInterruptAndStartMeasurement();

    float tofData[2] = {0.f, tofDis};
    Serial.println("tofDis");
    roveComm.write(RC_RAMANBOARD_POSITION_DATA_ID, 2, tofData);

    //Limit switches 
    uint8_t limitSwitchData[2] = {digitalRead(LIMIT_SW1), digitalRead(LIMIT_SW2)};
    roveComm.write(RC_RAMANBOARD_LIMITSWITCH_DATA_ID, 2, limitSwitchData);

    telemetryCounter = millis();
  }
}

//Watchdog Stuff
void estop() {
    if (!watchdogOverride) {
      smoco.stopAndReset();
    }
}


void feedWatchdog() {
    Watchdog.begin(estop, WATCHDOG_TIMEOUT);
}


byte identifyTofAddress()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    // The Wire.beginTransmission function sends an I2C start condition and the
    // device address. An address of 0 is a General Call address.
    Wire.beginTransmission(address);
    
    // The endTransmission function sends an I2C stop condition and releases the bus.
    // It also returns an error code (0 for success)
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Device found at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
      return address;
      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  return address;
}