// Touch screen support code by maxpautsch, merged on 1/10/17
// https://github.com/maxpautsch

// Define TOUCH_CS in the user setup file to enable this code

// A demo is provided in the examples/Generic folder

// Additions by Bodmer to double sample, use Z value to improve detection reliability,
// and to correct rotation handling

// See license in root directory

// Default pressure threshold
#ifndef Z_THRESHOLD
  #define Z_THRESHOLD 350 // Threshold for validating touches
#endif

/***************************************************************************************
** Function name:           begin_touch_read_write
** Description:             Starts the transaction and selects the touch controller
***************************************************************************************/
inline void TFT_eSPI::begin_touch_read_write(void){
  DMA_BUSY_CHECK;
  CS_H; // Ensure CS is high
  #if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS)
    if (locked) {
      locked = false;
      spi.beginTransaction(SPISettings(SPI_TOUCH_FREQUENCY, MSBFIRST, SPI_MODE0));
    }
  #else
    spi.setFrequency(SPI_TOUCH_FREQUENCY);
  #endif
  SET_BUS_READ_MODE;
  T_CS_L;
}

/***************************************************************************************
** Function name:           end_touch_read_write
** Description:             Ends the transaction and deselects the touch controller
***************************************************************************************/
inline void TFT_eSPI::end_touch_read_write(void){
  T_CS_H;
  #if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS)
    if (!inTransaction) {
      if (!locked) {
        locked = true;
        spi.endTransaction();
      }
    }
  #else
    spi.setFrequency(SPI_FREQUENCY);
  #endif
}

/***************************************************************************************
** Function name:           getTouchRaw
** Description:             Reads raw touch position. Always returns true.
***************************************************************************************/
uint8_t TFT_eSPI::getTouchRaw(uint16_t *x, uint16_t *y) {
  uint16_t tmp;

  begin_touch_read_write();
  
  // Start YP sample request for x position, read 4 times and keep last sample
  spi.transfer(0xd0);
  spi.transfer(0);
  spi.transfer(0xd0);
  spi.transfer(0);
  spi.transfer(0xd0);
  spi.transfer(0);
  spi.transfer(0xd0);
  
  tmp = spi.transfer(0);
  tmp = tmp << 5;
  tmp |= 0x1f & (spi.transfer(0x90) >> 3);

  *x = tmp;

  // Start XP sample request for y position, read 4 times and keep last sample
  spi.transfer(0);
  spi.transfer(0x90);
  spi.transfer(0);
  spi.transfer(0x90);
  spi.transfer(0);
  spi.transfer(0x90);
  
  tmp = spi.transfer(0);
  tmp = tmp << 5;
  tmp |= 0x1f & (spi.transfer(0) >> 3);

  *y = tmp;

  end_touch_read_write();

  return true;
}

/***************************************************************************************
** Function name:           getTouchRawZ
** Description:             Reads raw pressure (Z value) on touchpad and returns it
***************************************************************************************/
uint16_t TFT_eSPI::getTouchRawZ(void) {
  begin_touch_read_write();

  int16_t tz = 0xFFF;
  spi.transfer(0xb0);
  tz += spi.transfer16(0xc0) >> 3;
  tz -= spi.transfer16(0x00) >> 3;

  end_touch_read_write();

  if (tz == 4095) tz = 0;

  return (uint16_t)tz;
}

/***************************************************************************************
** Function name:           validTouch
** Description:             Reads validated touch position. Returns false if not pressed.
***************************************************************************************/
#define _RAWERR 20 // Allowed deadband error in successive position samples
uint8_t TFT_eSPI::validTouch(uint16_t *x, uint16_t *y, uint16_t threshold) {
  uint16_t x_tmp, y_tmp, x_tmp2, y_tmp2;

  // Wait until pressure stabilizes to debounce
  uint16_t z1 = 1;
  uint16_t z2 = 0;
  while (z1 > z2) {
    z2 = z1;
    z1 = getTouchRawZ();
    delay(1);
  }

  if (z1 <= threshold) return false;
    
  getTouchRaw(&x_tmp, &y_tmp);

  delay(1); 
  if (getTouchRawZ() <= threshold) return false;

  delay(2);
  getTouchRaw(&x_tmp2, &y_tmp2);

  if (abs(x_tmp - x_tmp2) > _RAWERR) return false;
  if (abs(y_tmp - y_tmp2) > _RAWERR) return false;
  
  *x = x_tmp;
  *y = y_tmp;
  
  return true;
}

/***************************************************************************************
** Function name:           getTouch
** Description:             Reads calibrated position. Returns false if not pressed.
***************************************************************************************/
uint8_t TFT_eSPI::getTouch(uint16_t *x, uint16_t *y, uint16_t threshold) {
  uint16_t x_tmp, y_tmp;
  
  if (threshold < 20) threshold = 20;
  if (_pressTime > millis()) threshold = 20;

  uint8_t n = 5;
  uint8_t valid = 0;
  while (n--) {
    if (validTouch(&x_tmp, &y_tmp, threshold)) valid++;
  }

  if (valid < 1) {
    _pressTime = 0;
    return false;
  }
  
  _pressTime = millis() + 50;

  convertRawXY(&x_tmp, &y_tmp);

  if (x_tmp >= _width || y_tmp >= _height) return false;

  _pressX = x_tmp;
  _pressY = y_tmp;
  *x = _pressX;
  *y = _pressY;
  return valid;
}

/***************************************************************************************
** Function name:           convertRawXY
** Description:             Converts raw touch x,y values to screen coordinates
***************************************************************************************/
void TFT_eSPI::convertRawXY(uint16_t *x, uint16_t *y) {
  uint16_t x_tmp = *x, y_tmp = *y, xx, yy;

  if (!touchCalibration_rotate) {
    xx = (x_tmp - touchCalibration_x0) * _width / touchCalibration_x1;
    yy = (y_tmp - touchCalibration_y0) * _height / touchCalibration_y1;
    if (touchCalibration_invert_x) xx = _width - xx;
    if (touchCalibration_invert_y) yy = _height - yy;
  } else {
    xx = (y_tmp - touchCalibration_x0) * _width / touchCalibration_x1;
    yy = (x_tmp - touchCalibration_y0) * _height / touchCalibration_y1;
    if (touchCalibration_invert_x) xx = _width - xx;
    if (touchCalibration_invert_y) yy = _height - yy;
  }
  *x = xx;
  *y = yy;
}

/***************************************************************************************
** Function name:           calibrateTouch
** Description:             Generates calibration parameters for the touchscreen
***************************************************************************************/
void TFT_eSPI::calibrateTouch(uint16_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size) {
  int16_t values[] = {0,0,0,0,0,0,0,0};
  uint16_t x_tmp, y_tmp;

  for (uint8_t i = 0; i < 4; i++) {
    fillRect(0, 0, size+1, size+1, color_bg);
    fillRect(0, _height-size-1, size+1, size+1, color_bg);
    fillRect(_width-size-1, 0, size+1, size+1, color_bg);
    fillRect(_width-size-1, _height-size-1, size+1, size+1, color_bg);

    if (i == 5) break; // Used to clear the arrows
    
    switch (i) {
      case 0: // Top-left
        drawLine(0, 0, 0, size, color_fg);
        drawLine(0, 0, size, 0, color_fg);
        drawLine(0, 0, size , size, color_fg);
        break;
      case 1: // Bottom-left
        drawLine(0, _height-size-1, 0, _height-1, color_fg);
        drawLine(0, _height-1, size, _height-1, color_fg);
        drawLine(size, _height-size-1, 0, _height-1 , color_fg);
        break;
      case 2: // Top-right
        drawLine(_width-size-1, 0, _width-1, 0, color_fg);
        drawLine(_width-size-1, size, _width-1, 0, color_fg);
        drawLine(_width-1, size, _width-1, 0, color_fg);
        break;
      case 3: // Bottom-right
        drawLine(_width-size-1, _height-size-1, _width-1, _height-1, color_fg);
        drawLine(_width-size-1, _height-1, _width-1, _height-1, color_fg);
        drawLine(_width-1, _height-size-1, _width-1, _height-1, color_fg);
        break;
    }

    while (!getTouchRaw(&x_tmp, &y_tmp));

    values[i * 2] = x_tmp;
    values[i * 2 + 1] = y_tmp;
  }

  parameters[0] = values[0]; // Top left X
  parameters[1] = values[2] - values[0]; // X delta left to right
  parameters[2] = values[1]; // Top left Y
  parameters[3] = values[5] - values[1]; // Y delta top to bottom

  parameters[4] = (values[0] + values[2]) >> 1; // Average X center
  parameters[5] = (values[1] + values[5]) >> 1; // Average Y center
  parameters[6] = 1; // No rotate
  parameters[7] = 0; // No invert x or y
}

/***************************************************************************************
** Function name:           setTouch
** Description:             Imports a previously calibrated touch screen parameters
***************************************************************************************/
void TFT_eSPI::setTouch(uint16_t *parameters) {
  touchCalibration_x0 = parameters[0];
  touchCalibration_x1 = parameters[1];
  touchCalibration_y0 = parameters[2];
  touchCalibration_y1 = parameters[3];

  touchCalibration_rotate = parameters[6];
  touchCalibration_invert_x = parameters[7] & 0x01;
  touchCalibration_invert_y = parameters[7] & 0x02;
}

