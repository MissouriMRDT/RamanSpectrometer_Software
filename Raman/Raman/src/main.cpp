#include "Raman_Software.h"
#include "RamanData.h"

float dataMax = 0;

void setup()
{
  for (int i = 0; i < PIXEL_COUNT; i++)
  {
    backgroundSub[i] = 0;
  }
  
  pinMode(24, OUTPUT);
  digitalWrite(24, HIGH);
  pinMode(40, OUTPUT);
  digitalWrite(40, HIGH);
  pinMode(ST, OUTPUT);
  digitalWrite(ST, LOW);
  //ADC testing stuff
  pinMode(A10, OUTPUT);
  pinMode(A11, OUTPUT);
  pinMode(33, OUTPUT);//CVNST

  digitalWrite(33, HIGH);
  digitalWrite(A10, HIGH);


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
  tofSensor->setXShutPin(17);

  tofSensor->begin();
  tofSensor->VL53L4CX_Off();
  Serial.println(tofSensor->InitSensor(tofAddress));
  tofSensor->VL53L4CX_ClearInterruptAndStartMeasurement();

  //Smoco setup:
  smoco.setSoftLimitPosition(INT32_MIN, INT32_MAX);
  smoco.setRampRate(200);
  smoco.configAngleConversion(0, STEPS_PER_INCH);

  for(float datum : fakeData)
  {
    if (datum > dataMax)
      dataMax = datum;
  }
}

void loop() {
  //Process ping data                                                                         
  receiveCAN();

  doRoveComm();

  doGantryButtons();

  if (millis() - telemetryCounter > TELLEMENTARY_MILLIS)
  {
    doTelementary();
    telemetryCounter = millis();
  }
}

//Watchdog Stuff
void estop() {
    if (!watchdogOverride) {
      smoco.driveOpenLoop(0);
      Serial.printf("%d: WATCHDOG\n", millis());
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


void doRoveComm()
{
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
    if (packet.i32data[0]%10 != 1)
      integrationCycles = ceil(((packet.i32data[0] < 10 ? 10 : packet.i32data[0])) / 10.);
    else
      backgroundScan = true;
    cmosSensor.setStartCycles(packet.i32data[0]);
    static bool eSwitch = false;
    cmosSensor.read(eSwitch);
    eSwitch = !eSwitch;
    waitForADC = true;
    break;

  case RC_RAMANBOARD_INSTRUMENTSAXIS_DATA_ID:
    //Open loop gantry driving
    smoco.driveOpenLoop(packet.i16data[0]);
    feedWatchdog();

    break;
  case RC_RAMANBOARD_WATCHDOGOVERRIDE_DATA_ID:
    //override watchdog, why?
    watchdogOverride = packet.i8data[0];
    break;
  case RC_RAMANBOARD_CALIBRATEENCODER_DATA_ID:
  {
    //calibrate smoco: THIS WILL MOVE THE GANTRY AND PREVENT INPUTS UNTIL IT IS DONE
    uint8_t oldWatchdogOverride = watchdogOverride;
    watchdogOverride = 1;
    smoco.calibratePosition(INT16_MAX * 0.8, 0);
    uint32_t timeout = millis() + 10000;
    while (!smoco.getCalibrated() && millis() < timeout) {
      delay(500);
      Serial.println("CALIBRATING");
    }
    watchdogOverride = oldWatchdogOverride;
    break;
  }
  case RC_RAMANBOARD_LIMITSWITCHOVERRIDE_DATA_ID:
    //ignore limits, why?
    uint8_t limitData = packet.i8data[0];
    smoco.setIgnoreLimit(limitData & 1, limitData & 2);
    break;
  }
}


void doGantryButtons()
{
  //Gantry Button Inputs
  if (!digitalRead(CAN_SW1) && digitalRead(CAN_SW2))
  {
    smoco.driveOpenLoop(INT16_MAX * .8);                                                                                                                                                                                                                                                                                                                                                                                                                                                 
    Serial.println("Driving Forward");
    feedWatchdog();
  }
  else if (digitalRead(CAN_SW1) && !digitalRead(CAN_SW2))
  {
    smoco.driveOpenLoop(INT16_MIN/2);
    Serial.println("Driving Backward");
    feedWatchdog();
  }

  //If both buttons are down then assume TOF calibration state
  if (!digitalRead(CAN_SW1) && !digitalRead(CAN_SW2))
    calibrationState = true;

}


void doTelementary()
{
  //Tof Telementary Data Retrieval
  VL53L4CX_MultiRangingData_t multiRangingData;

  if (!tofFailed)
  {
    tofSensor->VL53L4CX_GetMultiRangingData(&multiRangingData);
    //Serial.println((float)((multiRangingData.RangeData[1].RangeMilliMeter > multiRangingData.RangeData[0].RangeMilliMeter ? multiRangingData.RangeData[1].RangeMilliMeter : multiRangingData.RangeData[0].RangeMilliMeter) - tofCallibrationOffset));
    tofSensor->VL53L4CX_ClearInterruptAndStartMeasurement();
    
    float positionData[2] = {smoco.getAngle(), MM_TO_INCH * (float)((multiRangingData.RangeData[1].RangeMilliMeter > multiRangingData.RangeData[0].RangeMilliMeter ? multiRangingData.RangeData[1].RangeMilliMeter : multiRangingData.RangeData[0].RangeMilliMeter) - tofCallibrationOffset)};
    if (calibrationState)
    {
      tofCallibrationOffset = positionData[1];
      calibrationState = false;
    }

    Serial.println(positionData[0]);

    roveComm.write(RC_RAMANBOARD_POSITION_DATA_ID, RC_RAMANBOARD_POSITION_DATA_COUNT, positionData);
  }

  //SMOCO ping
  smoco.ping();
  uint16_t smocoPingData = smoco.getPingTime(); 
  roveComm.write(RC_RAMANBOARD_SMOCOPING_DATA_ID, 1, &smocoPingData);


  //SMOCO limits
  uint8_t limitData = (smoco.getLimitSwitchForward()) | (smoco.getLimitSwitchReverse() ? (1 << 1) : 0);
  roveComm.write(RC_RAMANBOARD_LIMITSWITCH_DATA_ID, 1, &limitData);

  //Sensor Data
  adcDataP = cmosSensor.getData();
  if (waitForADC == true && adcDataP != nullptr)
  {
    doRamanTelementary();
  }
}


void doRamanTelementary()
{
  //clamp adc outputs
  for (int i = 0; i < PIXEL_COUNT; i++)
  {
    adcDataP[i] = adcDataP[i] > 17000 ? 17000 : adcDataP[i] < 0 ? 0 : adcDataP[i];
  }
  //Background subtraction scan
  if (backgroundScan)
  {
    backgroundScan = false;
    for (int i = 0; i < PIXEL_COUNT; i++)
    {
      backgroundSub[i] = adcDataP[i];
    }

    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART1_DATA_ID, 512 , &adcDataP[0]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART2_DATA_ID, 512, &adcDataP[512]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART3_DATA_ID, 512, &adcDataP[1024]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART4_DATA_ID, 512, &adcDataP[1536]);
  }
  //integration cycles.
  else if (integrationCount < integrationCycles - 1)
  {
    for (int i = 0; i < PIXEL_COUNT; i++)
    {
      adcDataP[i] -= backgroundSub[i];
    }
    integrationCount++;
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART1_DATA_ID, 512 , &adcDataP[0]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART2_DATA_ID, 512, &adcDataP[512]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART3_DATA_ID, 512, &adcDataP[1024]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART4_DATA_ID, 512, &adcDataP[1536]);
    cmosSensor.read(false);
  }
  //Final cycle.
  else
  {
    integrationCount = 0;
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART1_DATA_ID, 512 , &adcDataP[0]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART2_DATA_ID, 512, &adcDataP[512]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART3_DATA_ID, 512, &adcDataP[1024]);
    roveComm.write(RC_RAMANBOARD_RAMANREADING_PART4_DATA_ID, 512, &adcDataP[1536]);
    cmosSensor.clearData();
    Serial.println("Sending...");

    waitForADC = false;
  }
  //sei();
}