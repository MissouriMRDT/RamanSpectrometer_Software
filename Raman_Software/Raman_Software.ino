#include "Raman_Software.h"


void setup(){
    Serial.begin(BAUDRATE);
    Serial.println("setup");

    pinMode(CLK_OUT, OUTPUT);
    pinMode(ROG, OUTPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(RED, OUTPUT);
    pinMode(VOUT, INPUT);
    pinMode(CLK_IN, INPUT);

    initADC();
    
    analogWriteFrequency(CLK_OUT, CLK_FREQUENCY);
    analogWrite(CLK_OUT, 0);//50% duty cycle
    Serial.println(CLK_TIMEPERIOD*CLK_REPETITIONS*1000.0);

    
    Serial.println("RoveComm Initializing...");
    RoveComm.begin(RC_ARMBOARD_FIRSTOCTET, RC_ARMBOARD_SECONDOCTET, RC_ARMBOARD_THIRDOCTET, RC_ARMBOARD_FOURTHOCTET, &TCPServer);
    Serial.println("Complete");
}


void loop(){
    //watchdog setup to turn off lights if there is no communication???

    rovecomm_packet packet = RoveComm.read();
    switch (packet.data_id) {

        case 8000:
        {
            readCCD();
            RoveComm.write(8010, 500, &pixelArray[0]);
            RoveComm.write(8011, 500, &pixelArray[500]);
            RoveComm.write(8012, 500, &pixelArray[1000]);
            RoveComm.write(8013, 500, &pixelArray[1500]);
            RoveComm.write(8014, 48, &pixelArray[2000]);

        }

        case 8001:
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
	
    // 4.121 us
	// 10 bit conversion (17 clocks) plus 20 clocks for input settling
    // async clock
	ADC1_CFG = ADC_CFG_MODE(1) | ADC_CFG_ADSTS(2) | ADC_CFG_ADLSMP | ADC_CFG_ADIV(1) | ADC_CFG_ADICLK(3) | ADC_CFG_ADHSC;

    // 3.385 us
    // 8 bit conversion (17 clocks) plus 8 clocks for input settling
    // async clock
	// ADC1_CFG = ADC_CFG_MODE(0) | ADC_CFG_ADSTS(3) | ADC_CFG_ADIV(1) | ADC_CFG_ADICLK(3) | ADC_CFG_ADHSC;
  
    // calibrate
	ADC1_GC = ADC_GC_CAL;
	while (ADC1_GC & ADC_GC_CAL);
}


uint16_t pixel_index = 0;
void CLK_ISR() {
    if(pixel_index >= 33 && pixel_index < 2048+33){
      delay(CLK_TIMEPERIOD * 0.1);
        pixelArray[pixel_index-33] = analogRead(CLK_IN);
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
    delay(CLK_TIMEPERIOD*CLK_REPETITIONS*1000.0);
    detachInterrupt(digitalPinToInterrupt(CLK_IN));
    analogWrite(CLK_OUT, 0);

}