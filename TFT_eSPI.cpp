#include "TFT_eSPI.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

// Constructor for hardware SPI
TFT_eSPI::TFT_eSPI() {
  _cs   = TFT_CS;
  _dc   = TFT_DC;
  _rst  = TFT_RST;
  _mosi = TFT_MOSI;
  _miso = TFT_MISO;
  _sclk = TFT_SCLK;
  _spi  = spi0;
}

// Initialize SPI and GPIO
void TFT_eSPI::spi_begin() {
  // Set the CS, DC, and RST pins to output mode
  gpio_init(_cs);
  gpio_set_dir(_cs, GPIO_OUT);
  gpio_put(_cs, 1);  // Ensure CS is high

  gpio_init(_dc);
  gpio_set_dir(_dc, GPIO_OUT);
  gpio_put(_dc, 1);  // Set DC high by default

  if (_rst >= 0) {
    gpio_init(_rst);
    gpio_set_dir(_rst, GPIO_OUT);
    gpio_put(_rst, 1);  // Set RST high
  }

  // Initialize SPI
  spi_init(_spi, 40000000);  // Initialize SPI with a default frequency of 40MHz
  spi_set_format(_spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
  
  gpio_set_function(_mosi, GPIO_FUNC_SPI);
  gpio_set_function(_sclk, GPIO_FUNC_SPI);

  if (_miso >= 0) {
    gpio_set_function(_miso, GPIO_FUNC_SPI);
  }
}

// Function to begin an SPI transaction
void TFT_eSPI::spi_beginTransaction() {
  gpio_put(_cs, 0);  // Assert CS pin (active low)
}

// Function to end an SPI transaction
void TFT_eSPI::spi_endTransaction() {
  gpio_put(_cs, 1);  // Deassert CS pin
}

// Function to send a byte over SPI
void TFT_eSPI::spi_transfer(uint8_t data) {
  spi_write_blocking(_spi, &data, 1);
}

// Function to read a byte over SPI
uint8_t TFT_eSPI::spi_transfer(uint8_t data) {
  uint8_t result;
  spi_write_blocking(_spi, &data, 1);  // Write data
  spi_read_blocking(_spi, 0x00, &result, 1);  // Read the response
  return result;
}

// Send a command to the TFT
void TFT_eSPI::writecommand(uint8_t c) {
  gpio_put(_dc, 0);  // Command mode
  spi_beginTransaction();
  spi_transfer(c);
  spi_endTransaction();
  gpio_put(_dc, 1);  // Back to data mode
}

// Send data to the TFT
void TFT_eSPI::writedata(uint8_t d) {
  spi_beginTransaction();
  spi_transfer(d);
  spi_endTransaction();
}

// Reset the display
void TFT_eSPI::reset(void) {
  if (_rst >= 0) {
    gpio_put(_rst, 0);  // Assert reset
    sleep_ms(50);       // Wait 50ms
    gpio_put(_rst, 1);  // Deassert reset
    sleep_ms(50);       // Wait 50ms
  }
}

// Initialization of the display
void TFT_eSPI::begin(void) {
  // Reset display
  reset();

  // Your display-specific initialization code here
}

// Set the rotation of the display
void TFT_eSPI::setRotation(uint8_t m) {
  spi_beginTransaction();
  writecommand(0x36);  // Memory Access Control

  rotation = m % 4;  // Limit the rotation value to 0-3

  switch (rotation) {
    case 0:
      writedata(TFT_MADCTL_MX | TFT_MADCTL_BGR);
      _width  = TFT_WIDTH;
      _height = TFT_HEIGHT;
      break;
    case 1:
      writedata(TFT_MADCTL_MV | TFT_MADCTL_BGR);
      _width  = TFT_HEIGHT;
      _height = TFT_WIDTH;
      break;
    case 2:
      writedata(TFT_MADCTL_MY | TFT_MADCTL_BGR);
      _width  = TFT_WIDTH;
      _height = TFT_HEIGHT;
      break;
    case 3:
      writedata(TFT_MADCTL_MX | TFT_MADCTL_MY | TFT_MADCTL_MV | TFT_MADCTL_BGR);
      _width  = TFT_HEIGHT;
      _height = TFT_WIDTH;
      break;
  }
  spi_endTransaction();
}

// Push a single pixel color
void TFT_eSPI::drawPixel(int32_t x, int32_t y, uint32_t color) {
  if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

  spi_beginTransaction();

  setAddrWindow(x, y, x + 1, y + 1);

  writedata(color >> 8);
  writedata(color);

  spi_endTransaction();
}

// Set the address window for drawing
void TFT_eSPI::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  spi_beginTransaction();

  writecommand(0x2A);  // Column addr set
  writedata(x0 >> 8);
  writedata(x0 & 0xFF);
  writedata(x1 >> 8);
  writedata(x1 & 0xFF);

  writecommand(0x2B);  // Row addr set
  writedata(y0 >> 8);
  writedata(y0 & 0xFF);
  writedata(y1 >> 8);
  writedata(y1 & 0xFF);

  writecommand(0x2C);  // Write to RAM

  spi_endTransaction();
}

// Draw a filled rectangle
void TFT_eSPI::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
  if ((x >= _width) || (y >= _height)) return;

  int32_t x2 = x + w - 1;
  int32_t y2 = y + h - 1;

  if ((x2 < 0) || (y2 < 0)) return;

  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x2 >= _width)  w = _width  - x;
  if (y2 >= _height) h = _height - y;

  spi_beginTransaction();

  setAddrWindow(x, y, x2, y2);

  uint16_t pixelCount = w * h;
  while (pixelCount--) {
    writedata(color >> 8);
    writedata(color);
  }

  spi_endTransaction();
}

// Fill the screen with a color
void TFT_eSPI::fillScreen(uint32_t color) {
  fillRect(0, 0, _width, _height, color);
}

// Draw a horizontal line
void TFT_eSPI::drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color) {
  if ((x >= _width) || (y >= _height)) return;

  int32_t x2 = x + w - 1;
  if (x2 < 0) return;

  if (x < 0) { w += x; x = 0; }
  if (x2 >= _width) w = _width - x;

  spi_beginTransaction();

  setAddrWindow(x, y, x2, y);

  while (w--) {
    writedata(color >> 8);
    writedata(color);
  }

  spi_endTransaction();
}

// Draw a vertical line
void TFT_eSPI::drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color) {
  if ((x >= _width) || (y >= _height)) return;

  int32_t y2 = y + h - 1;
  if (y2 < 0) return;

  if (y < 0) { h += y; y = 0; }
  if (y2 >= _height) h = _height - y;

  spi_beginTransaction();

  setAddrWindow(x, y, x, y2);

  while (h--) {
    writedata(color >> 8);
    writedata(color);
  }

  spi_endTransaction();
}
// Push a single color pixel to the TFT display at the set address window
void TFT_eSPI::pushColor(uint16_t color) {
  spi_beginTransaction();

  writedata(color >> 8);
  writedata(color);

  spi_endTransaction();
}

// Push an array of colors to the TFT display
void TFT_eSPI::pushColors(uint16_t *data, uint32_t len) {
  spi_beginTransaction();

  while (len--) {
    writedata(*data >> 8);
    writedata(*data++);
  }

  spi_endTransaction();
}

// Push a block of 8-bit color data to the display
void TFT_eSPI::pushColors(uint8_t *data, uint32_t len) {
  spi_beginTransaction();

  while (len--) {
    writedata(*data++);
  }

  spi_endTransaction();
}

// Set the brightness of the display using PWM
void TFT_eSPI::setBrightness(uint8_t brightness) {
  // Assuming the use of a GPIO pin for backlight control via PWM
  // Configure the GPIO pin for PWM output (replace BACKLIGHT_PIN with actual pin)
  gpio_set_function(BACKLIGHT_PIN, GPIO_FUNC_PWM);
  uint slice_num = pwm_gpio_to_slice_num(BACKLIGHT_PIN);

  // Set the PWM frequency and duty cycle
  pwm_set_wrap(slice_num, 255);  // 8-bit resolution
  pwm_set_gpio_level(BACKLIGHT_PIN, brightness);
  pwm_set_enabled(slice_num, true);
}

// Read data from the TFT display
uint8_t TFT_eSPI::readcommand8(uint8_t cmd) {
  uint8_t data;

  spi_beginTransaction();

  gpio_put(_dc, 0);  // Command mode
  spi_transfer(cmd);
  gpio_put(_dc, 1);  // Data mode

  // Now we need to read the data
  gpio_put(_cs, 0);  // Assert CS
  data = spi_transfer(0x00);  // Dummy write to receive data
  gpio_put(_cs, 1);  // Deassert CS

  spi_endTransaction();

  return data;
}

// Read 16-bit data from the display
uint16_t TFT_eSPI::read16(uint8_t cmd) {
  uint8_t high, low;

  spi_beginTransaction();

  gpio_put(_dc, 0);  // Command mode
  spi_transfer(cmd);
  gpio_put(_dc, 1);  // Data mode

  gpio_put(_cs, 0);  // Assert CS
  high = spi_transfer(0x00);  // Read high byte
  low = spi_transfer(0x00);   // Read low byte
  gpio_put(_cs, 1);  // Deassert CS

  spi_endTransaction();

  return (high << 8) | low;
}

// Read the ID of the display (for compatibility checks)
uint32_t TFT_eSPI::readID(void) {
  spi_beginTransaction();

  writecommand(0x04);  // Read Display Identification Information

  uint32_t r = read16(0x04);
  r <<= 16;
  r |= read16(0x04);

  spi_endTransaction();

  return r;
}

// Invert the display colors
void TFT_eSPI::invertDisplay(bool i) {
  spi_beginTransaction();
  writecommand(i ? 0x21 : 0x20);  // 0x21 = invert colors, 0x20 = normal mode
  spi_endTransaction();
}

// Write a 16-bit command to the display
void TFT_eSPI::writecommand16(uint16_t c) {
  gpio_put(_dc, 0);  // Command mode
  spi_beginTransaction();
  spi_transfer(c >> 8);  // Send high byte
  spi_transfer(c & 0xFF);  // Send low byte
  spi_endTransaction();
  gpio_put(_dc, 1);  // Data mode
}

// Write a 16-bit data value to the display
void TFT_eSPI::writedata16(uint16_t d) {
  spi_beginTransaction();
  spi_transfer(d >> 8);  // Send high byte
  spi_transfer(d & 0xFF);  // Send low byte
  spi_endTransaction();
}

// Draw a circle outline
void TFT_eSPI::drawCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color) {
  int32_t f = 1 - r;
  int32_t ddF_x = 1;
  int32_t ddF_y = -2 * r;
  int32_t x = 0;
  int32_t y = r;

  drawPixel(x0  , y0+r, color);
  drawPixel(x0  , y0-r, color);
  drawPixel(x0+r, y0  , color);
  drawPixel(x0-r, y0  , color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);
  }
}

// Fill a circle
void TFT_eSPI::fillCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color) {
  drawFastVLine(x0, y0-r, 2*r+1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}

// Helper function for filling circle quadrants
void TFT_eSPI::fillCircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t corners, int32_t delta, uint32_t color) {
  int32_t f = 1 - r;
  int32_t ddF_x = 1;
  int32_t ddF_y = -2 * r;
  int32_t x = 0;
  int32_t y = r;
  int32_t px = x;
  int32_t py = y;

  delta++;
  
  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    if (x < (y + 1)) {
      if (corners & 1) drawFastVLine(x0+x, y0-y, 2*y+delta, color);
      if (corners & 2) drawFastVLine(x0-x, y0-y, 2*y+delta, color);
    }
    if (y != py) {
      if (corners & 1) drawFastVLine(x0+py, y0-px, 2*px+delta, color);
      if (corners & 2) drawFastVLine(x0-py, y0-px, 2*px+delta, color);
      py = y;
    }
    px = x;
  }
}


// Explanation:
// TFT_eSPI Constructor: Initializes the GPIO pins and SPI interface.
// spi_begin: Configures the GPIO pins and initializes SPI with the Raspberry Pi Pico API.
// spi_beginTransaction & spi_endTransaction: Manage the CS pin for SPI communication.
// spi_transfer: Handles sending and receiving data over SPI.
// writecommand & writedata: Send commands or data to the display.
// reset: Resets the TFT display by toggling the reset pin.
// begin: Placeholder for further initialization steps specific to the display.
// setRotation: Sets the rotation of the display and adjusts the width and height accordingly.
// drawPixel: Draws a single pixel on the screen at the specified coordinates.
// setAddrWindow: Defines the area of the screen where data will be written.
// fillRect: Draws a filled rectangle with the specified dimensions and color.
// fillScreen: Fills the entire screen with a single color.
// drawFastHLine: Draws a horizontal line on the display.
// drawFastVLine: Draws a vertical line on the display.
// pushColor, pushColors: These functions are used to push pixel data to the TFT display.
// setBrightness: Adjusts the display brightness using PWM (if available).
// readcommand8, read16, readID: Functions to read data or the ID from the TFT display.
// invertDisplay: Inverts the display colors.
// writecommand16, writedata16: Write 16-bit commands or data to the display.
// drawCircle, fillCircle, fillCircleHelper: Functions to draw and fill circles on the display.

