#include "Raman_Software.h"
#include "RamanData.h"

float dataMax = 0;

void calibrateSMOCO();

void setup()
{
  //Serial setup, get rid of the while for normal opperation 
  Serial.begin(115200);

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

  tofSensor->begin();
  tofSensor->VL53L4CX_Off();
  Serial.println(tofSensor->InitSensor(tofAddress));
  tofSensor->VL53L4CX_ClearInterruptAndStartMeasurement();

  //Smoco setup:                                                                                                                                                                                                                                                                                                                                             
  calibrateSMOCO();
  smoco.setSoftLimitPosition(0, INT32_MAX);
  //smoco.calibratePosition((INT16_MIN / 4), 0);
  smoco.setRampRate(200);


  for(float datum : fakeData)
  {
    if (datum > dataMax)
      dataMax = datum;
  }
}

void loop() {
  //Process ping data
  //receiveCAN();

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
    uint16_t temp[2048];  
    for (int i = 0; i < 2048; i++)
    {
        temp[i] = fakeData[(int)(i * (2478.0 / 2048.0))] / dataMax * 1023;
    }
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART1_DATA_ID, 512, &temp[0]);
    delay(100);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART2_DATA_ID, 512, &temp[512]);
    delay(100);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART3_DATA_ID, 512, &temp[1024]);
    delay(100);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART4_DATA_ID, 512, &temp[1536]);
    delay(100);
    
    //waitForADC = true;
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
    smoco.configIgnoreLimits(limitData & 1, limitData & 2);
    break;
  }
  

  //Gantry Button Inputs
  if (!digitalRead(CAN_SW1) && digitalRead(CAN_SW2))
  {
    smoco.driveOpenLoop(INT16_MAX/2);
    Serial.println("Driving Forward");
    feedWatchdog();
  }
  else if (digitalRead(CAN_SW1) && !digitalRead(CAN_SW2))
  {
    smoco.driveOpenLoop(INT16_MIN/2);
    Serial.println("Driving Backward");
    feedWatchdog();
  }
  else
    smoco.driveOpenLoop(instrumentGantrySpeed);

  //If both buttons are down then assume TOF calibration state
  if (!digitalRead(CAN_SW1) && !digitalRead(CAN_SW2))
    calibrationState = true;

  if (millis() - telemetryCounter > 500)
  {
    //Tof Telementary Data Retrieval
    VL53L4CX_MultiRangingData_t multiRangingData;

    if (!tofFailed)
    {
      tofSensor->VL53L4CX_GetMultiRangingData(&multiRangingData);

      tofSensor->VL53L4CX_ClearInterruptAndStartMeasurement();

      float positionData[2] = {smoco.getPosition() * INCHES_PER_STEP, (float)((multiRangingData.RangeData[1].RangeMilliMeter > multiRangingData.RangeData[0].RangeMilliMeter ? multiRangingData.RangeData[1].RangeMilliMeter : multiRangingData.RangeData[0].RangeMilliMeter) - tofCallibrationOffset)};
      if (calibrationState)
      {
        tofCallibrationOffset = positionData[1];
        calibrationState = false;
      }

      roveComm.write(RC_RAMANBOARD_POSITION_DATA_ID, RC_RAMANBOARD_POSITION_DATA_COUNT, positionData);
    }

    //Limit switches 
    //uint8_t limitSwitchData[2] = {digitalRead(LIMIT_SW1), digitalRead(LIMIT_SW2)};
    //roveComm.write(RC_RAMANBOARD_LIMITSWITCH_DATA_ID, 2, limitSwitchData);

    //SMOCO ping
    smoco.ping();
    uint16_t smocoPingData = smoco.getPingTime(); 
    roveComm.write(RC_RAMANBOARD_SMOCOPING_DATA_ID, 1, &smocoPingData);

    //SMOCO limits
    uint8_t limitData = (smoco.getLimitSwitchA()) | (smoco.getLimitSwitchB() ? (1 << 1) : 0);
    roveComm.write(RC_RAMANBOARD_LIMITSWITCH_DATA_ID, 1, &limitData);

    //Sensor Data
    adcDataP = cmosSensor.getData();
    if (waitForADC == true && adcDataP != nullptr)
    {
      sei();
      roveComm.write(RC_RAMANBOARD_RAMANREADING_PART1_DATA_ID, 512, &adcDataP[0]);
      roveComm.write(RC_RAMANBOARD_RAMANREADING_PART2_DATA_ID, 512, &adcDataP[512]);
      roveComm.write(RC_RAMANBOARD_RAMANREADING_PART3_DATA_ID, 512, &adcDataP[1024]);
      roveComm.write(RC_RAMANBOARD_RAMANREADING_PART4_DATA_ID, 512, &adcDataP[1536]);

      waitForADC = false;
    }
    telemetryCounter = millis();
  }
}

//Watchdog Stuff
void estop() {
    if (!watchdogOverride) {
      instrumentGantrySpeed = 0;
      Serial.printf("%d: WATCHDOG\n", millis());
    }
}


void feedWatchdog() {
    Watchdog.begin(estop, WATCHDOG_TIMEOUT);
}


void calibrateSMOCO()
{
  while (!smoco.getLimitSwitchA())
  {
    smoco.driveOpenLoop(1000);
  }
  smoco.driveOpenLoop(0);
  smoco.calibratePosition(INT16_MIN / 4, 0);
}


byte identifyTofAddress()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  //check for a valid I2C connection at every adress. 
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Device found at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
      tofFailed = false;
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
  {
    tofFailed = true;
    Serial.println("No I2C devices found\n");
  }
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