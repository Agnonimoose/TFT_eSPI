#ifndef _TFT_eSPIH_
#define _TFT_eSPIH_

#include <stdint.h>
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

// Define default pin numbers (you may need to adjust these for your setup)
#define TFT_CS     5   // Chip select control pin
#define TFT_DC     6   // Data/Command control pin
#define TFT_RST    7   // Reset pin (can connect to Arduino RESET pin)
#define TFT_MOSI   19  // SPI MOSI pin
#define TFT_MISO   16  // SPI MISO pin (optional, depending on display)
#define TFT_SCLK   18  // SPI clock pin

#define BACKLIGHT_PIN 21 // Pin for controlling backlight brightness (via PWM)

// Define the dimensions of your display (change as needed)
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Color definitions for easier coding
#define TFT_BLACK       0x0000
#define TFT_BLUE        0x001F
#define TFT_RED         0xF800
#define TFT_GREEN       0x07E0
#define TFT_CYAN        0x07FF
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0xFFE0
#define TFT_WHITE       0xFFFF

class TFT_eSPI {
public:
    TFT_eSPI(); // Constructor

    // Initialize the display
    void begin();

    // Set the display rotation
    void setRotation(uint8_t r);

    // Push a single color pixel to the display
    void drawPixel(int32_t x, int32_t y, uint32_t color);

    // Fill the screen with a single color
    void fillScreen(uint32_t color);

    // Draw a filled rectangle
    void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);

    // Draw a horizontal line
    void drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color);

    // Draw a vertical line
    void drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color);

    // Push a single color to the current pixel position
    void pushColor(uint16_t color);

    // Push an array of colors to the display
    void pushColors(uint16_t *data, uint32_t len);
    void pushColors(uint8_t *data, uint32_t len);

    // Set the display brightness
    void setBrightness(uint8_t brightness);

    // Read a command or data from the display
    uint8_t readcommand8(uint8_t cmd);
    uint16_t read16(uint8_t cmd);
    uint32_t readID(void);

    // Invert the display colors
    void invertDisplay(bool i);

    // Write 16-bit command or data to the display
    void writecommand16(uint16_t cmd);
    void writedata16(uint16_t data);

    // Draw and fill circles
    void drawCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color);
    void fillCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color);
    void fillCircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t corners, int32_t delta, uint32_t color);

private:
    // SPI and GPIO related functions
    void spi_begin();
    void spi_beginTransaction();
    void spi_endTransaction();
    void spi_transfer(uint8_t data);
    uint8_t spi_transfer(uint8_t data);

    // Display control functions
    void reset(void);
    void writecommand(uint8_t c);
    void writedata(uint8_t d);
    void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

    // SPI instance and control pins
    spi_inst_t *_spi;
    int8_t _cs, _dc, _rst, _mosi, _miso, _sclk;

    // Display dimensions
    uint16_t _width, _height;
    uint8_t rotation;
};

#endif


// Explanation of the Header File:
// Class Declaration: The TFT_eSPI class contains all the necessary methods and properties for controlling the TFT display.
// Constructor & Initialization: The constructor and begin method are defined for initializing the display and setting up the necessary SPI and GPIO configurations.
// SPI Communication Methods: Functions like spi_begin, spi_beginTransaction, spi_endTransaction, and spi_transfer handle the SPI communication with the display.
// Display Control Methods: Functions such as setRotation, drawPixel, fillScreen, fillRect, etc., provide the interface for drawing on the display.
// Utility Methods: Functions for setting brightness, inverting the display, and reading data or commands are also included.
// GPIO and Pin Definitions: Pin definitions are included to set the default GPIO pins for SPI communication and control.