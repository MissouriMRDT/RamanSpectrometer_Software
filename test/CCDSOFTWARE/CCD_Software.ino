#include <cstdint>

// Fluorometer Pins
#define CCD_IN A10
#define SH 2
#define ICG 3
#define MASTER_CLK 1
#define CLK_IN 4
#define DATA_CLK 5
#define DATA_CLK_IN 8

#define CONTROL_LED_265 27
#define CONTROL_LED_275 38
#define CONTROL_LED_280 39
#define CONTROL_LED_310 40
#define CONTROL_LED_365 41

// Variables
uint16_t ccd_buff[3648];
int ccd_buff_index = 0;



void setup() {
    pinMode(MASTER_CLK, OUTPUT);
    pinMode(CLK_IN, INPUT);

    analogWriteResolution(8);
    analogWriteFrequency(MASTER_CLK, 1000000);
    analogWriteFrequency(DATA_CLK, 1000000 / 4);
    analogWrite(MASTER_CLK, 128);
    analogWrite(DATA_CLK, 32);
}


void loop() {
    readCCD();
}


void readCCD() {
    while (!digitalRead(DATA_CLK_IN));

    digitalWrite(SH, LOW);
    digitalWrite(ICG, HIGH);
    delayMicroseconds(2);
    digitalWrite(SH, HIGH);
    delayMicroseconds(5);
    digitalWrite(ICG, LOW);

    ccd_buff_index = 0;
    attachInterrupt(digitalPinToInterrupt(DATA_CLK_IN), ccd_isr, RISING);
    delay(20);
    detachInterrupt(digitalPinToInterrupt(DATA_CLK_IN));
}


void ccd_isr() {
    if (ccd_buff_index >= 32 && ccd_buff_index < (32 + 3648)) {
        ccd_buff[ccd_buff_index - 32] = analogRead(CCD_IN);
    }
    ccd_buff_index++;
}