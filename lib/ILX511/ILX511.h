#ifndef ILX511_H
#define ILX511_H

#include <stdint.h>


class ILX511 {
private:
    uint8_t m_CLK_pin;
    uint8_t m_ROG_pin;
    uint8_t m_VOUT_pin;
    
    uint32_t m_CLK_freq = 0;
    uint32_t m_minIntegrationTime = 0;
    uint32_t m_integrationTime = 0;
    
public:
    ILX511(uint8_t CLK_pin, uint8_t ROG_pin, uint8_t VOUT_pin);
    
    void init(uint32_t CLK_freq = 100000);
    
    void setIntegrationTime(uint32_t time_ms);
    
    void read(uint16_t data[2048]);

private:
    static uint8_t s_CLKPin;
    static bool s_CLKToggle;
    static uint8_t s_VOUTPin;
    static uint16_t s_pixelIndex;
    static uint16_t *s_pixelArray;
    static void isr(void);
    static IntervalTimer ReadTimer;

};



#endif /* ILX511_H */