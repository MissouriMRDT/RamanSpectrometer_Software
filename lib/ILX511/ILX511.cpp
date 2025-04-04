#include "ILX511.h"
#include <Arduino.h>


/**
 * Datasheet: https://www.spectralproducts.com/pdf/ILX511.pdf
 * We have tied the SHSW switch to ground so we are running in S/H (Sample and Hold) mode
 * 
 * Relevant timing diagram is in the datasheet.
 * 
 */


// Constants from the datasheet:

#define MAX_CLK_FREQ 2'000'000 // 2MHz


/*
 *  The ILX511 begins reading when it sees this signal:
 *
 *  ROG __________|                |__________
 *                |\              /|
 *                | \ <-- t7 --> / |
 *                |  \__________/  |
 *                |                |
 * CLK       |____|________________|____|
 *          /|    |                |    |\
 *         / | t5 |                | t9 | \
 *     ___/  |    |                |    |  \_____
 * 
 * The rise time is expected to be around 10ns. However, the level shifter mosfet is slow
 * mosfet used: https://www.onsemi.com/pdf/datasheet/bss138-d.pdf
 * It has a worst-case turn-on delay+rise of 23ns and turn-off delay+fall of 50ns, so we might need to correct for it
 */


#define CLK_t5 3000 // ns
#define ROG_t7 (5000-50) //ns
#define CLK_t9 3000 // ns

/*
 * The ILX511 then reads for 2088+ clock pulses
 * First it reads out 33 dummy values D0..=D32
 * D12..=D30 should read in as 0
 * 
 * Then it reads 2048 picture elements S1..=S2048
 * The reading is output on the falling edge of the clock signal after a delay of max 280 ns
 * 
 * Then it reads out 6 more dummy values D33..=D38
 * The ILX511 is then reset with another t5 -> t7 -> t9 signal
 */

#define PIXEL_COUNT 2048
#define DUMMY_COUNT 33
#define CLK_REPETITIONS 2088

#define NUM_READINGS 4

ILX511::ILX511(uint8_t CLK_pin, uint8_t ROG_pin, uint8_t VOUT_pin) {
  m_CLK_pin = CLK_pin;
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

    pinMode(m_ROG_pin, OUTPUT);
    pinMode(m_CLK_pin, OUTPUT);
    pinMode(m_VOUT_pin, INPUT);

    digitalWrite(m_CLK_pin, LOW);
    digitalWrite(m_ROG_pin, HIGH);
}

uint8_t ILX511::s_CLKPin;
bool ILX511::s_CLKToggle;
uint8_t ILX511::s_VOUTPin;
uint16_t ILX511::s_pixelIndex;
uint16_t *ILX511::s_pixelArray;
IntervalTimer ILX511::ReadTimer;

void ILX511::isr() {
    if (s_CLKToggle) {
        digitalWriteFast(s_CLKPin, LOW);
    } else {
        digitalWriteFast(s_CLKPin, HIGH);
    }

    if (s_CLKToggle) {
        if (s_pixelIndex >= PIXEL_COUNT + DUMMY_COUNT) {
            return;
        }
    
        if (s_pixelIndex >= DUMMY_COUNT) {
            delayNanoseconds(280); // rise time of VOUT
            s_pixelArray[s_pixelIndex - DUMMY_COUNT] = analogRead(s_VOUTPin);
        }
        s_pixelIndex++;
    }
}

void ILX511::read(uint16_t data[2048]){
    // ADC configuration
    analogReadAveraging(4); // This would make the ADC theoretically 4x slower
    analogReadRes(10); // 8, 10, or 12

    // CCD behaves strangely for first couple cycles. Discard the first ones and record only the last.
    for (uint16_t i = NUM_READINGS; i != 0; i--) {

        // Initial state
        digitalWriteFast(m_CLK_pin, LOW);
        digitalWriteFast(m_ROG_pin, HIGH);
        delayMicroseconds(100); // arbitrary delay

        // ROG and CLK start pulse
        digitalWriteFast(m_CLK_pin, HIGH);
        digitalWriteFast(m_ROG_pin, HIGH);
        delayNanoseconds(CLK_t5);

        digitalWriteFast(m_CLK_pin, HIGH);
        digitalWriteFast(m_ROG_pin, LOW);
        delayNanoseconds(ROG_t7);

        digitalWriteFast(m_CLK_pin, HIGH);
        digitalWriteFast(m_ROG_pin, HIGH);
        delayNanoseconds(CLK_t9);

        // Interrupt requires global variables, pass members into global equivalents
        s_CLKPin = m_CLK_pin;
        s_CLKToggle = false;
        s_VOUTPin = m_VOUT_pin;
        s_pixelIndex = 0;
        s_pixelArray = data;

        // Clock and take reading
        ReadTimer.begin(isr, ((1.0f / m_CLK_freq) * 1'000'000) / 2); // Divide by 2 for clock toggle
        
        /*
        ??? weird Malacki code
        if (i == 2) {
            // Before final reading, wait full integration time
            delay(m_integrationTime);
        } else {
            // Other readings are discarded, so wait as short as possible
            delay(m_minIntegrationTime);
        }*/
        delay(m_integrationTime);
        ReadTimer.end();
    }
}


void ILX511::setIntegrationTime(uint32_t time_ms) {
    if (time_ms < m_minIntegrationTime) {
        time_ms = m_minIntegrationTime;
    }

    m_integrationTime = time_ms;
}