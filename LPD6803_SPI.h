#if (ARDUINO >= 100)
    #include <Arduino.h>
#else
    #include <WProgram.h>
    #include <pins_arduino.h>
#endif

class LPD6803_SPI {
    public:
        LPD6803_SPI(uint16_t n); // Use SPI hardware; specific pins only:
        void
            begin(uint8_t divider),
            show(void),
            setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
            setPixelColor(uint16_t n, uint16_t c);
        uint16_t
            numPixels(void);

    private:
        uint16_t
            numLEDs;
        void
            startSPI(uint8_t divider);
};