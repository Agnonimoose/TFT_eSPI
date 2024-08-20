#ifndef _TFT_eSPI_BUTTON_H_
#define _TFT_eSPI_BUTTON_H_

#include <stdint.h>
#include "TFT_eSPI.h"

class TFT_eSPI_Button
{
 public:
  TFT_eSPI_Button(void);

  // "Classic" initButton() uses center & size
  void initButton(TFT_eSPI *gfx, int16_t x, int16_t y,
                  uint16_t w, uint16_t h, uint16_t outline, uint16_t fill,
                  uint16_t textcolor, const char *label, uint8_t textsize);

  // New/alt initButton() uses upper-left corner & size
  void initButtonUL(TFT_eSPI *gfx, int16_t x1, int16_t y1,
                    uint16_t w, uint16_t h, uint16_t outline, uint16_t fill,
                    uint16_t textcolor, const char *label, uint8_t textsize);

  // Adjust text datum and x, y deltas
  void setLabelDatum(int16_t x_delta, int16_t y_delta, uint8_t datum = MC_DATUM);

  // Draw the button
  void drawButton(bool inverted = false, const char *long_name = nullptr);

  // Check if coordinates are within button bounds
  bool contains(int16_t x, int16_t y);

  // Set button state
  void press(bool p);
  
  // Button state checks
  bool isPressed();
  bool justPressed();
  bool justReleased();

 private:
  TFT_eSPI *_gfx;
  int16_t _x1, _y1;       // Coordinates of top-left corner of button
  int16_t _xd, _yd;       // Button text datum offsets (relative to the center of the button)
  uint16_t _w, _h;        // Width and height of button
  uint8_t _textsize, _textdatum; // Text size multiplier and text datum for button
  uint16_t _outlinecolor, _fillcolor, _textcolor;
  char _label[10];        // Button text is 9 chars maximum unless long_name is used

  bool currstate, laststate; // Button states
};

#endif // _TFT_eSPI_BUTTON_H_
