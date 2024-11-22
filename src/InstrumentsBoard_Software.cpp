#include "InstrumentsBoard_Software.h"
#include <Arduino.h>

void setup() {
  // Serial Debugger
  Serial.begin(115200);
  Serial.println("Instruments Setup");

  miniSpec.init();
  ramanCCD.init(40000);
  
  pinMode(GREEN_LASER, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);
  digitalWrite(GREEN_LASER, LOW);
  digitalWrite(WHITE_LED, LOW);

  Serial.println("RoveComm Initializing...");
  RoveComm.begin(RC_INSTRUMENTSBOARD_FIRSTOCTET, RC_INSTRUMENTSBOARD_SECONDOCTET, RC_INSTRUMENTSBOARD_THIRDOCTET, RC_INSTRUMENTSBOARD_FOURTHOCTET, &TCPServer);
  Serial.println("Complete");
}


void loop() {

  rovecomm_packet packet = RoveComm.read();
  switch (packet.data_id) {
    
    // Toggle LEDs
    case RC_INSTRUMENTSBOARD_ENABLELEDS_DATA_ID:
    {
      uint8_t data = ((uint8_t*) packet.data)[0];
      digitalWrite(GREEN_LASER, (data & 1<<0));
      digitalWrite(WHITE_LED, (data & 1<<1));
      break;
    }
    
    // Request Raman
    case RC_INSTRUMENTSBOARD_REQUESTRAMANREADING_DATA_ID:
    {
      uint32_t data = ((uint32_t*) packet.data)[0];
      ramanCCD.setIntegrationTime(data);

      Serial.print("Raman: ");
      Serial.println(data);
      
      uint16_t pixels[2048];
      ramanCCD.read(pixels);

      RoveComm.write(RC_INSTRUMENTSBOARD_RAMANREADING_PART1_DATA_ID, 500, &pixels[0]);
      RoveComm.write(RC_INSTRUMENTSBOARD_RAMANREADING_PART2_DATA_ID, 500, &pixels[500]);
      RoveComm.write(RC_INSTRUMENTSBOARD_RAMANREADING_PART3_DATA_ID, 500, &pixels[1000]);
      RoveComm.write(RC_INSTRUMENTSBOARD_RAMANREADING_PART4_DATA_ID, 500, &pixels[1500]);
      RoveComm.write(RC_INSTRUMENTSBOARD_RAMANREADING_PART5_DATA_ID, 48, &pixels[2000]);
      break;
    }

    // Request Reflectance
    case (RC_INSTRUMENTSBOARD_REQUESTREFLECTANCEREADING_DATA_ID):
    {
      uint32_t data = ((uint32_t*) packet.data)[0];
      miniSpec.setIntegrationTime(data * 1000);

      Serial.print("Reflectance: ");
      Serial.println(data);

      uint8_t video[288];
      miniSpec.read(video);
      RoveComm.write(RC_INSTRUMENTSBOARD_REFLECTANCEREADING_DATA_ID, RC_INSTRUMENTSBOARD_REFLECTANCEREADING_DATA_COUNT, video);
      break;
    }

    // Request Temperature
    case (RC_INSTRUMENTSBOARD_REQUESTTEMPERATURE_DATA_ID):
    {
      int measurement = analogRead(TEMP);
      long mapped = map(measurement, TEMP_ADC_MIN, TEMP_ADC_MAX, TEMP_MIN, TEMP_MAX);
      mapped += 6; // it kept getting 6 degrees Celsius off so I'll just add 6 degrees
      int8_t temperature = (uint8_t) constrain(mapped, -128, 127); // to be safe and avoid overflow
      RoveComm.write(RC_INSTRUMENTSBOARD_TEMPERATURE_DATA_ID, RC_INSTRUMENTSBOARD_TEMPERATURE_DATA_COUNT, temperature); // this should probably be a float
      break;
    }
  }
  
}

// float analogMap(uint16_t measurement, uint16_t fromADC, uint16_t toADC, float fromAnalog, float toAnalog) {
//   float slope = (toAnalog - fromAnalog) / (toADC - fromADC);
//   return (measurement - fromADC) * slope + fromAnalog;
// }
