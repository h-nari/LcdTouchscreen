#ifndef _lcd_touchscreen_h
#define _lcd_touchscreen_h

#include "XPT2046_Touchscreen.h"
#include "Adafruit_GFX.h"
#include "Params.h"

#define PARAM_CALIBRATION  "calib"

class CalibData {
 public:
  uint16_t lcd[2];		// LCDのX座標、Ｙ座標
  uint16_t tp[2];		// TouchPanelの値、X座標、Ｙ座標
};

class LcdTouchscreen {
 protected:
  XPT2046_Touchscreen m_ts;
  int16_t m_a[2];
  int16_t m_b[2][2];
  Adafruit_GFX *m_dpy;
  uint16_t m_fgColor;
  uint16_t m_bgColor;
  
 public:
  LcdTouchscreen(uint8_t cs_pin = 16);

  bool begin();
  bool touched() {return m_ts.touched();}
  bool getPoint(int16_t *pX, int16_t *pY, uint16_t *pZ=NULL);
  bool getRawPoint(int16_t *pX, int16_t *pY, uint16_t *pZ=NULL);
  bool setCalibration(const CalibData *aData, uint16_t nData);
  bool calibrate(Adafruit_GFX *dpy, uint16_t fgColor, uint16_t bgColor);
  void getCalibrationString(char *buf, size_t size);
  bool saveCalibrationData(Params *params);
  bool loadCalibrationData(Params *params);
  
 protected:
  bool getCalibrationData(uint16_t xs, uint16_t ys, CalibData *data);

};


#endif /* _lcd_touchscreen_h */
