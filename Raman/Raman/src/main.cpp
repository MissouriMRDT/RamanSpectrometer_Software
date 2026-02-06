#include "Raman_Software.h"


void setup()
{
  //Serial setup, get rid of the while for normal opperation 
  Serial.begin(115200);

  while(!Serial);

  //Setup Outputs
  pinMode(FAN_OUT, OUTPUT);
  digitalWrite(FAN_OUT, LOW);
  pinMode(LASER_OUT, OUTPUT);
  digitalWrite(LASER_OUT, LOW);
  

  //Inputs:
  pinMode(LIMIT_SW1, INPUT_PULLDOWN);
  pinMode(LIMIT_SW2, INPUT_PULLDOWN);
  pinMode(CAN_SW1, INPUT_PULLUP);
  pinMode(CAN_SW2, INPUT_PULLUP);

  //CAN INNIT
  ATAN_T4_CAN.begin(acanSettings);
  pinMode(CAN_STDBY1, OUTPUT);
  digitalWrite(CAN_STDBY1, LOW);
  //pinMode(CAN_STDBY2, OUTPUT);
  //digitalWrite(CAN_STDBY2, LOW);

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
  tofSensor->VL53L4CX_ClearInterruptAndStartMeasurement();

  //Smoco setup:
  smoco.setLowPassSmoothingFactor(UINT16_MAX);                                                                                                                                                                                                                                                                                                                                                                                       
  smoco.setSoftLimitPosition(INT32_MIN, INT32_MAX);
  smoco.calibratePosition((INT16_MIN / 4), 0);
}

void loop() {
  //Process ping data
  receiveCAN();

  //Check for RoveComm Packets:
  roveComm.read(packet); 

  //process RoveComm Packets:
  switch (packet.dataId)
  { 
  //Laser Toggle
  case RC_RAMANBOARD_LASER_DATA_ID:
    
    digitalWrite(FAN_OUT, packet.i8data[0]);
    digitalWrite(LASER_OUT, packet.i8data[0]);
    Serial.println("LASER");
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
    instrumentGantrySpeed = packet.i16data[0];
    feedWatchdog();

    break;
  case RC_RAMANBOARD_WATCHDOGOVERRIDE_DATA_ID:
    watchdogOverride = packet.i8data[0];


    break;
  case RC_RAMANBOARD_CALIBRATEENCODER_DATA_ID:
    smoco.calibratePosition(INT16_MIN / 4, 0);
    break;
  case RC_RAMANBOARD_LIMITSWITCHOVERRIDE_DATA_ID:
    uint8_t limitData = packet.i8data[0];
    smoco.m_ignoreLimit = limitData;
    break;
  }
  

  //Gantry Button Inputs
  if (!digitalRead(CAN_SW1) && digitalRead(CAN_SW2))
  {
    smoco.driveOpenLoop(INT16_MAX/4);
    feedWatchdog();
    Serial.println("button1");
  }
  else if (digitalRead(CAN_SW1) && !digitalRead(CAN_SW2))
  {
    smoco.driveOpenLoop(INT16_MIN/4);
    feedWatchdog();
  }
  else
    smoco.driveOpenLoop(instrumentGantrySpeed);

  //If both buttons are down then assume TOF callibration state
  if (!digitalRead(CAN_SW1) && !digitalRead(CAN_SW2))
    callibrationState = true;

  if (millis() - telemetryCounter > 500)
  {
    //Tof Telementary Data Retrieval
    VL53L4CX_MultiRangingData_t multiRangingData;
    float tofDis = FLT_MAX;

    tofSensor->VL53L4CX_GetMultiRangingData(&multiRangingData);

    tofSensor->VL53L4CX_ClearInterruptAndStartMeasurement();

    float tofData[2] = {0.f, (float)(multiRangingData.RangeData[0].RangeMilliMeter - tofCallibrationOffset)};
    Serial.printf("Tof Data: %dmm\n", multiRangingData.RangeData[0].RangeMilliMeter - tofCallibrationOffset);
    if (callibrationState)
    {
      Serial.println("hello");
      tofCallibrationOffset = multiRangingData.RangeData[0].RangeMilliMeter;
      callibrationState = false;
    }

    roveComm.write(RC_RAMANBOARD_POSITION_DATA_ID, 2, tofData);

    //Limit switches 
    uint8_t limitSwitchData[2] = {digitalRead(LIMIT_SW1), digitalRead(LIMIT_SW2)};
    roveComm.write(RC_RAMANBOARD_LIMITSWITCH_DATA_ID, 2, limitSwitchData);

    //SMOCO ping
    smoco.ping();
    uint16_t smocoPingData = smoco.m_pingTime; 
    roveComm.write(RC_RAMANBOARD_SMOCOPING_DATA_ID, 1, &smocoPingData);
    Serial.printf("Ping Data: %d\n", smocoPingData);


    //SMOCO limits
    uint8_t limitData = (smoco.m_limitSwitchA) | (smoco.m_limitSwitchB ? (1 << 1) : 0);
    roveComm.write(RC_ARMBOARD_LIMITSWITCH_DATA_ID, 1, &limitData);

    telemetryCounter = millis();
  }
}

//Watchdog Stuff
void estop() {
    if (!watchdogOverride) {
      instrumentGantrySpeed = 0;
      Serial.println("WATCHDOG");
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


void receiveCAN()
{
  CANMessage msg;
  while (ATAN_T4_CAN.available())
  {
    ATAN_T4_CAN.receive(msg);
    smoco.sync(msg);
  }

} 