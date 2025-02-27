#include "ILX511.h"
#include <Arduino.h>


#define NUM_READINGS 4
#define MAX_CLK_FREQ 200000 // Hz

#define CLK_t5 3000 // ns
#define ROG_t7 (5000-50) //ns
#define CLK_t9 3000 // ns

#define PIXEL_COUNT 2048
#define DUMMY_COUNT 33
#define CLK_REPETITIONS 2088


ILX511::ILX511(uint8_t CLK_OUT_pin, uint8_t CLK_IN_pin, uint8_t ROG_pin, uint8_t VOUT_pin) {
  m_CLK_OUT_pin = CLK_OUT_pin;
//   m_CLK_IN_pin = CLK_IN_pin;
  m_ROG_pin = ROG_pin;
  m_VOUT_pin = VOUT_pin;
}


void ILX511::init(uint32_t CLK_freq) {
    if (CLK_freq > MAX_CLK_FREQ) {
        CLK_freq = MAX_CLK_FREQ;
    }
    m_CLK_freq = CLK_freq;
    
    m_minIntegrationTime = CLK_REPETITIONS * 1.5 * 1000 / CLK_freq;
    if (m_integrationTime < m_minIntegrationTime) {
        m_integrationTime = m_integrationTime;
    }

    // pinMode(m_CLK_IN_pin, INPUT);
    pinMode(m_ROG_pin, OUTPUT);
    //analogWriteFrequency(m_CLK_OUT_pin, CLK_freq);
    //analogWrite(m_CLK_OUT_pin, 0);
}


uint16_t pixel_index;
uint16_t *pixelArray;
uint8_t VOUT_pin;


IntervalTimer ReadTimer;
uint8_t ILX511::s_readPin = 0;

void CLK_ISR() {
    digitalWriteFast(ILX511::s_readPin, HIGH);
    if (pixel_index >= PIXEL_COUNT + DUMMY_COUNT) {
        return;
    }

    if (pixel_index >= DUMMY_COUNT) {
        delayNanoseconds(70); // rise time of VOUT
        pixelArray[pixel_index - DUMMY_COUNT] = analogRead(VOUT_pin);
    }
    pixel_index++;

    delayNanoseconds(100);
    digitalWriteFast(ILX511::s_readPin, LOW);
}

void ILX511::read(uint16_t data[2048]){
    // ADC configuration
    analogReadAveraging(4);
    analogReadRes(10);

    // CCD behaves strangely for first couple cycles. Discard the first ones and record only the last.
    for (uint16_t i = NUM_READINGS; i != 0; i--) {
        // ROG and CLK start pulse
        pinMode(m_CLK_OUT_pin, OUTPUT);
        digitalWriteFast(m_CLK_OUT_pin, HIGH);
        digitalWriteFast(m_ROG_pin, HIGH);
        delayNanoseconds(CLK_t5);

        digitalWriteFast(m_CLK_OUT_pin, HIGH);
        digitalWriteFast(m_ROG_pin, LOW);
        delayNanoseconds(ROG_t7);

        digitalWriteFast(m_CLK_OUT_pin, HIGH);
        digitalWriteFast(m_ROG_pin, HIGH);
        delayNanoseconds(CLK_t9);

        // Interrupt requires global variables, pass members into global equivalents
        pixel_index = 0;
        pixelArray = data;
        VOUT_pin = m_VOUT_pin;
        // attachInterrupt(digitalPinToInterrupt(m_CLK_IN_pin), CLK_ISR, RISING);

        // Clock and take reading
        digitalWriteFast(m_ROG_pin, HIGH);
        s_readPin = m_CLK_OUT_pin;
        ReadTimer.begin(CLK_ISR, ((1.0f / m_CLK_freq) * 1000000));
        // analogWrite(m_CLK_OUT_pin, 128);
        
        /*
        if (i == 2) {
            // Before final reading, wait full integration time
            delay(m_integrationTime);
        } else {
            // Other readings are discarded, so wait as short as possible
            delay(m_minIntegrationTime);
        }*/
        delay(m_integrationTime);

        // analogWrite(m_CLK_OUT_pin, 0);
        // detachInterrupt(digitalPinToInterrupt(m_CLK_IN_pin));
        ReadTimer.end();
    }
}


void ILX511::setIntegrationTime(uint32_t time_ms) {
    if (time_ms < m_minIntegrationTime) {
        time_ms = m_minIntegrationTime;
    }

    m_integrationTime = time_ms;
}