#include <TimerOne.h>
#include "SPI.h"
#include "Neophob_LPD6803.h"

#define SPI_A(data) SPDR=data;
#define SPI_B while(!(SPSR & (1<<SPIF))); 
#define SPI_TRANSFER(data) { SPDR=data; while(!(SPSR & (1<<SPIF))); } 

// Example to control LPD6803-based RGB LED Modules in a strand or strip
/*****************************************************************************/

//some local variables, ised in isr
static uint8_t isDirty;
static uint16_t prettyUglyCopyOfNumPixels;
static uint16_t *pData;
static uint16_t *pDataStart;

// Constructor for use with hardware SPI (specific clock/data pins):
Neophob_LPD6803::Neophob_LPD6803(uint16_t n) {
  numLEDs = n;
  prettyUglyCopyOfNumPixels = n;  
  pixels = (uint16_t *)malloc(numLEDs);
  pDataStart = pixels; //huh, not sure
  isDirty = 0;    
  cpumax = 70;
}

//Interrupt routine.
//Frequency was set in setup(). Called once for every bit of data sent
//In your code, set global Sendmode to 0 to re-send the data to the pixels
//Otherwise it will just send clocks.
static void isr() {
  static unsigned char nState=1;
  static unsigned char indx=0;
  
  if(nState==1) {//send clock and check for color update (isDirty)  
    SPI_A(0); 
    if (isDirty==1) { //must we update the pixel value
      nState = 0;
      isDirty = 0;
      SPI_B; 
      SPI_A(0);
      indx = 0;
      pData = pDataStart; //reset index
      return;
    }
    SPI_B;
    SPI_A(0);
    return;
  }
  else {                        //feed out pixelbuffer    
    register uint16_t command;
    command = *(pData++);       //get current pixel
    SPI_B;                      //send 8bits
    SPI_A( (command>>8) & 0xFF);
    
    SPI_B;                      //send 8bits
    SPI_A( command & 0xFF);
    
    indx++;                     //are we done?
    if(indx >= prettyUglyCopyOfNumPixels) { 
      nState = 1;
    }

    return;
  } 
}


// Activate hard/soft SPI as appropriate:
void Neophob_LPD6803::begin(void) {
  startSPI();

  setCPUmax(cpumax);
  Timer1.attachInterrupt(isr);
}

void Neophob_LPD6803::setCPUmax(uint8_t max) {
  cpumax = max;

  // each clock out takes 20 microseconds max
  long time = 100;
  time *= 20;   // 20 microseconds per
  time /= max;    // how long between timers
  Timer1.initialize(time);
}


// Enable SPI hardware and set up protocol details:
void Neophob_LPD6803::startSPI(void) {
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV8);  // 2 MHz
  // WS2801 datasheet recommends max SPI clock of 2 MHz, and 50 Ohm
  // resistors on SPI lines for impedance matching.  In practice and
  // at short distances, 2 MHz seemed to work reliably enough without
  // resistors, and 4 MHz was possible with a 220 Ohm resistor on the
  // SPI clock line only.  Your mileage may vary.  Experiment!
  // SPI.setClockDivider(SPI_CLOCK_DIV4);  // 4 MHz
}

uint16_t Neophob_LPD6803::numPixels(void) {
  return numLEDs;
}


void Neophob_LPD6803::show(void) {
  isDirty=1; //flag to trigger redraw
}


void Neophob_LPD6803::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  uint16_t data;

  if (n > numLEDs) return;

  data = g & 0x1F;
  data <<= 5;
  data |= b & 0x1F;
  data <<= 5;
  data |= r & 0x1F;
  data |= 0x8000;

  pixels[n] = data;
}

//---
void Neophob_LPD6803::setPixelColor(uint16_t n, uint16_t c) {
  if (n > numLEDs) return;

  pixels[n] = 0x8000 | c;
}

