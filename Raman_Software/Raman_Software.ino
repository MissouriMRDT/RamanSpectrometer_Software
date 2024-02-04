#include "Raman_Software.h"


void setup(){
    Serial.begin(BAUDRATE);

    pinMode(CLK_OUT, OUTPUT);
    pinMode(ROG, OUTPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(RED, OUTPUT);
    pinMode(VOUT, INPUT);
    pinMode(CLK_IN, INPUT);

    initADC();
    
    analogWriteFrequency(CLK_OUT, CLK_FREQUENCY);
    analogWrite(CLK_OUT, 0);//50% duty cycle
    
    Serial.println("RoveComm Initializing...");
    RoveComm.begin(RC_RAMANSPECTROMETERBOARD_FIRSTOCTET, RC_RAMANSPECTROMETERBOARD_SECONDOCTET, RC_RAMANSPECTROMETERBOARD_THIRDOCTET, RC_RAMANSPECTROMETERBOARD_FOURTHOCTET, &TCPServer);
    Serial.println("Complete");
}


void loop(){
    //watchdog setup to turn off lights if there is no communication???

    readCCD();

    rovecomm_packet packet = RoveComm.read();
    switch (packet.data_id) {

        case RC_RAMANSPECTROMETERBOARD_REQUESTREADING_DATA_ID:
        {
            RoveComm.write(RC_RAMANSPECTROMETERBOARD_CCDREADING_PART1_DATA_ID, 500, &pixelArray[0]);
            RoveComm.write(RC_RAMANSPECTROMETERBOARD_CCDREADING_PART2_DATA_ID, 500, &pixelArray[500]);
            RoveComm.write(RC_RAMANSPECTROMETERBOARD_CCDREADING_PART3_DATA_ID, 500, &pixelArray[1000]);
            RoveComm.write(RC_RAMANSPECTROMETERBOARD_CCDREADING_PART4_DATA_ID, 500, &pixelArray[1500]);
            RoveComm.write(RC_RAMANSPECTROMETERBOARD_CCDREADING_PART5_DATA_ID, 48, &pixelArray[2000]);

        }

        case RC_RAMANSPECTROMETERBOARD_ENABLELEDS_DATA_ID:
        {
            uint8_t data = *((uint8_t*) packet.data);
            digitalWrite(GREEN, (data & 1<<0)? HIGH : LOW);
            digitalWrite(RED, (data & 1<<1)? HIGH : LOW);
        }

    }

}


void initADC() {
    // idk what this does
	  CCM_CCGR1 |= CCM_CCGR1_ADC1(CCM_CCGR_ON);
	  CCM_CCGR1 |= CCM_CCGR1_ADC2(CCM_CCGR_ON);
	
    // 4.121 us
	  // 10 bit conversion (17 clocks) plus 20 clocks for input settling
    // async clock
	  ADC1_CFG = ADC_CFG_MODE(1) | ADC_CFG_ADSTS(2) | ADC_CFG_ADLSMP | ADC_CFG_ADIV(1) | ADC_CFG_ADICLK(3) | ADC_CFG_ADHSC;
	  ADC2_CFG = ADC_CFG_MODE(1) | ADC_CFG_ADSTS(2) | ADC_CFG_ADLSMP | ADC_CFG_ADIV(1) | ADC_CFG_ADICLK(3) | ADC_CFG_ADHSC;

    // 3.385 us
    // 8 bit conversion (17 clocks) plus 8 clocks for input settling
    // async clock
	  // ADC1_CFG = ADC_CFG_MODE(0) | ADC_CFG_ADSTS(3) | ADC_CFG_ADIV(1) | ADC_CFG_ADICLK(3) | ADC_CFG_ADHSC;
    // ADC2_CFG = ADC_CFG_MODE(0) | ADC_CFG_ADSTS(3) | ADC_CFG_ADIV(1) | ADC_CFG_ADICLK(3) | ADC_CFG_ADHSC;
  
    // calibrate
	  ADC1_GC = ADC_GC_CAL;
	  ADC2_GC = ADC_GC_CAL;
	  while (ADC1_GC & ADC_GC_CAL);
	  while (ADC2_GC & ADC_GC_CAL);
}


uint16_t pixel_index = 0;
void CLK_ISR() {
    if(pixel_index >= 33 && pixel_index < 2048+33){
        delayNanoseconds(70);
        ADC2_HC0 = 3;
        while (!(ADC2_HS & ADC_HS_COCO0));
        pixelArray[pixel_index-33] = ADC2_R0; //analogRead(CLK_IN);
    }
    pixel_index++;
}

void readCCD(){
   
    pinMode(CLK_OUT, OUTPUT);
    digitalWrite(CLK_OUT, HIGH);
    digitalWrite(ROG, HIGH);
    delayNanoseconds(CLK_t5);

    digitalWrite(CLK_OUT, HIGH);
    digitalWrite(ROG, LOW);
    delayNanoseconds(ROG_t7);

    digitalWrite(CLK_OUT, HIGH);
    digitalWrite(ROG, HIGH);
    delayNanoseconds(CLK_t9);
    

    pixel_index = 0;
    attachInterrupt(digitalPinToInterrupt(CLK_IN), CLK_ISR, RISING);
    digitalWrite(ROG, HIGH);
    analogWrite(CLK_OUT, 128);
    delay(CLK_TIMEPERIOD*CLK_REPETITIONS*1000.0*1.5);
    detachInterrupt(digitalPinToInterrupt(CLK_IN));
    analogWrite(CLK_OUT, 0);

}