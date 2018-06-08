/*********************************************************************************
 * Example to control LPD6803-based RGB LED Modules in a strand or strip via SPI
 * by Michael Vogt / http://pixelinvaders.ch
 * 
 * Edited by Rudy Nurhadi for faster performance
 *
 * This Library is basically a copy and paste work and relies on work 
 * of Adafruit-WS2801-Library and FastSPI Library 
 *********************************************************************************/

#include "SPI.h"
#include "LPD6803_SPI.h"

// The SPI data register (SPDR) holds the byte which is about to be shifted out the MOSI line
#define SPI_LOAD_BYTE(data) SPDR=data
// Wait until last bytes is transmitted
#define SPI_WAIT_TILL_TRANSMITED while(!(SPSR & (1<<SPIF)))

static uint16_t index = 0;
static uint16_t *pixelData; //pointer to pixel buffer, we cannot access pixels form isr!

// Constructor for use with hardware SPI (specific clock/data pins):
LPD6803_SPI::LPD6803_SPI(uint16_t n) {
    numLEDs = n;  
    pixelData = (uint16_t *)malloc(n);

    // Clear buffer
    for (int i = 0; i < numLEDs; i++) {
        setPixelColor(i, 0);
    }
}

// Activate hard/soft SPI as appropriate:
void LPD6803_SPI::begin(uint8_t divider) {
    startSPI(divider);
}

// Enable SPI hardware and set up protocol details:
void LPD6803_SPI::startSPI(uint8_t divider) {
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(divider); 
    
    /* LPD6803 can handle a data/PWM clock of up to 25 MHz, and 50 Ohm
    resistors on SPI lines for impedance matching.  In practice and
    at short distances, 2 MHz seemed to work reliably enough without
    resistors, and 4 MHz was possible with a 220 Ohm resistor on the
    SPI clock line only.  Your mileage may vary.  Experiment! 
    
    Some available modes:
    SPI.setClockDivider(SPI_CLOCK_DIV2);   // 8 MHz
    SPI.setClockDivider(SPI_CLOCK_DIV8);   // 2 MHz
    SPI.setClockDivider(SPI_CLOCK_DIV16);  // 1 MHz  
    SPI.setClockDivider(SPI_CLOCK_DIV32);  // 0.5 MHz  
    SPI.setClockDivider(SPI_CLOCK_DIV64);  // 0.25 MHz */
    
    // SPI_A(0); //maybe, move at the end of the startSPI() method
}

uint16_t LPD6803_SPI::numPixels(void) {
    return numLEDs;
}

void LPD6803_SPI::show(void) {
    SPI.transfer(0);	
    SPI.transfer(0);	
    SPI.transfer(0);
    index = 0;

    while(index++ < numLEDs) {
        /* First shift in 32bit “0” as start frame, then shift in all data frame, start 
        frame and data frame both are shift by high-bit, every data is input on DCLK rising edge. */
        register uint16_t command;
        command = 0x8000 | pixelData[index]; // Get current pixel
        SPI.transfer((command >> 8) & 0xFF);
        SPI.transfer( command       & 0xFF);
    }
}

void LPD6803_SPI::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
    if (n > numLEDs)
        return;

    uint16_t data = g & 0x1F;
    data <<= 5;
    data |= b & 0x1F;
    data <<= 5;
    data |= r & 0x1F;
    data |= 0x8000; 

    pixelData[n] = data;
}

void LPD6803_SPI::setPixelColor(uint16_t n, uint16_t c) {
    if (n > numLEDs)
        return;

    pixelData[n] = 0x8000 | c; // The first bit of the color word must be set
}


