#include "SPIBetter.h"

uint16_t SPIClass::transfer14(uint16_t data) {
    Serial.println("muahahaha");
    uint32_t tcr = port().TCR;
    port().TCR = (tcr & 0xfffff000) | LPSPI_TCR_FRAMESZ(13);  // turn on 16 bit mode 
    port().TDR = data;		// output 16 bit data.
    while ((port().RSR & LPSPI_RSR_RXEMPTY)) ;	// wait while the RSR fifo is empty...
    port().TCR = tcr;	// restore back
    return port().RDR;
}