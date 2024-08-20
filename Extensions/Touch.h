#ifndef _TFT_eSPI_TOUCHH_
#define _TFT_eSPI_TOUCHH_

/***************************************************************************************
**                         Section for calibration variables
***************************************************************************************/
public:

uint16_t touchCalibration_x0 = 300;   // Calibration parameter 0
uint16_t touchCalibration_x1 = 3500;  // Calibration parameter 1
uint16_t touchCalibration_y0 = 350;   // Calibration parameter 2
uint16_t touchCalibration_y1 = 3600;  // Calibration parameter 3

uint8_t  touchCalibration_rotate = 0; // Calibration parameter 6 (screen rotation)
uint8_t  touchCalibration_invert_x = 0; // Calibration parameter 7 (invert x-axis)
uint8_t  touchCalibration_invert_y = 0; // Calibration parameter 7 (invert y-axis)

/***************************************************************************************
**                         Section for private touch functions
***************************************************************************************/
private:
inline void     begin_touch_read_write(void); // Start transaction, select touch controller
inline void     end_touch_read_write(void);   // End transaction, deselect touch controller

/***************************************************************************************
**                         Section for public touch functions
***************************************************************************************/
public:

uint8_t  getTouchRaw(uint16_t *x, uint16_t *y); // Read raw touch coordinates
uint16_t getTouchRawZ(void); // Read raw touch pressure (Z-axis)
uint8_t  validTouch(uint16_t *x, uint16_t *y, uint16_t threshold = Z_THRESHOLD); // Validate touch

uint8_t  getTouch(uint16_t *x, uint16_t *y, uint16_t threshold = Z_THRESHOLD); // Read calibrated touch coordinates
void     convertRawXY(uint16_t *x, uint16_t *y); // Convert raw touch coordinates to screen coordinates

void     calibrateTouch(uint16_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size); // Calibrate touch screen
void     setTouch(uint16_t *parameters); // Import touch calibration parameters

#endif // _TFT_eSPI_TOUCHH_
