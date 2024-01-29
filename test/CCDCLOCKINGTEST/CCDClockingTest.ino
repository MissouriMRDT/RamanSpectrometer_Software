#include <stdint.h>

#define BTN_PIN 13
#define SH 0
#define ICG 1
#define M_CLK 2
#define V_OUT 24

#define M_CLK_PERIOD (1000) // 500 Typical, 1250 Max
#define LOOP_DELAY (M_CLK_PERIOD/2)

#define SH_PERIOD (4000*4*M_CLK_PERIOD) //(20000)
#define SH_TIME_HIGH (5000)

#define ICG_PERIOD (4000*4*M_CLK_PERIOD)
#define ICG_TIME_LOW (5000 + SH_TIME_HIGH)

#define SH_CNT_RISE (500/LOOP_DELAY)
#define SH_CNT_FALL (SH_CNT_RISE + SH_TIME_HIGH/LOOP_DELAY)
#define SH_CNT_MAX (SH_PERIOD/LOOP_DELAY)

#define ICG_CNT_FALL (0)
#define ICG_CNT_RISE (ICG_CNT_FALL + ICG_TIME_LOW/LOOP_DELAY)
#define ICG_CNT_MAX (ICG_PERIOD/LOOP_DELAY)

#define NUM_CCD_ELEMENTS 3648
#define CCD_DATA_SIZE 3695
/*
  0: undef data
  1-16: D0-D15, dummy outputs
  17-29: D16-D28, light shield outputs
  30-32: D29-D31, 3 elements, idk what they are
  33-3681: Actual data elements
  3682-3695: D32-D45, dummy outputs
*/
int16_t ccd_data[3695];
int16_t* ccd_results = (ccd_data+32);


void setup() {
  initADC();
  Serial.begin(115200);
  
  pinMode(BTN_PIN, INPUT);
  pinMode(SH, OUTPUT);
  pinMode(ICG, OUTPUT);
  pinMode(M_CLK, OUTPUT);
  pinMode(V_OUT, INPUT);
  
  /*
  Serial.print("LOOP_DELAY: ");
  Serial.println(LOOP_DELAY);

  Serial.print("ICG_RISE: ");
  Serial.println(ICG_CNT_RISE);
  Serial.print("ICG_FALL: ");
  Serial.println(ICG_CNT_FALL);
  Serial.print("ICG_MAX: ");
  Serial.println(ICG_CNT_MAX);

  Serial.print("SH_RISE: ");
  Serial.println(SH_CNT_RISE);
  Serial.print("SH_FALL: ");
  Serial.println(SH_CNT_FALL);
  Serial.print("SH_MAX: ");
  Serial.println(SH_CNT_MAX);
  */

  // Initial values
  digitalWrite(M_CLK, HIGH);
  digitalWrite(ICG, LOW);
  digitalWrite(SH, HIGH);
  delay(1000);
}


bool last_btn = 0;
void loop() {
  bool btn = digitalRead(BTN_PIN);
  if (btn && !last_btn) {
    ccd_read();
    
    for (uint16_t i = 0; i < NUM_CCD_ELEMENTS; i++) {
      Serial.print(ccd_results[i]);
      Serial.print(", ");
    }
    Serial.println();
  }
  last_btn = btn;
}

void initADC() {
  // idk what this does
	CCM_CCGR1 |= CCM_CCGR1_ADC1(CCM_CCGR_ON);
	
  // 4.121 us
	// 10 bit conversion (17 clocks) plus 20 clocks for input settling
  // async clock
	// ADC1_CFG = ADC_CFG_MODE(1) | ADC_CFG_ADSTS(2) | ADC_CFG_ADLSMP | ADC_CFG_ADIV(1) | ADC_CFG_ADICLK(3) | ADC_CFG_ADHSC;

  // 3.385 us
  // 8 bit conversion (17 clocks) plus 8 clocks for input settling
  // async clock
	ADC1_CFG = ADC_CFG_MODE(0) | ADC_CFG_ADSTS(3) | ADC_CFG_ADIV(1) | ADC_CFG_ADICLK(3) | ADC_CFG_ADHSC;
  
  // calibrate
	ADC1_GC = ADC_GC_CAL;
	while (ADC1_GC & ADC_GC_CAL);
}


void ccd_read() {
  uint32_t icg_cnt = 0;
  uint32_t sh_cnt = 0;
  bool m_clk_edge = 1;
  bool data_start = 0;
  uint16_t data_state = 0;

  uint8_t completion_state = 0;

  do {
    if (sh_cnt == SH_CNT_RISE) {
      digitalWrite(SH, LOW);
      if (completion_state >= 2) completion_state = 3;
    } else if (sh_cnt == SH_CNT_FALL) {
      digitalWrite(SH, HIGH);
    } else if (sh_cnt == SH_CNT_MAX) {
      sh_cnt = 0;
    }
    sh_cnt++;

    if (icg_cnt == ICG_CNT_RISE) {
      digitalWrite(ICG, LOW);
      data_start = 1;
      completion_state++;
    } else if (icg_cnt == ICG_CNT_FALL) {
      digitalWrite(ICG, HIGH);
    } else if (icg_cnt == ICG_CNT_MAX) {
      icg_cnt = 0;
    }
    icg_cnt++;

    if (m_clk_edge) {
      digitalWrite(M_CLK, LOW);
      if (data_start) {
        if ((data_state%4 == 0) && (data_state < CCD_DATA_SIZE*4)) {
          ccd_data[data_state>>2] = (ADC1_HS & ADC_HS_COCO0)? ADC1_R0 : -1;
          ADC1_HC0 = 1;
        }
        data_state++;
      }
    } else {
      digitalWrite(M_CLK, HIGH);
    }
    
    m_clk_edge = !m_clk_edge;

    delayNanoseconds(LOOP_DELAY-87);
  } while (completion_state < 3);

}