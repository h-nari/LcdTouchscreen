#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "XPT2046_Touchscreen.h"
#include "LcdTouchscreen.h"
#include "Params.h"
#include <EEPROM.h>

#define TFT_CS	2
#define	TFT_DC	15
#define TP_CS 	16

Adafruit_ILI9341 tft(TFT_CS,TFT_DC);
LcdTouchscreen ts(TP_CS);
uint16_t c1 = ILI9341_GREEN;
uint16_t c2 = ILI9341_YELLOW;
uint16_t c3 = ILI9341_RED;
uint16_t bg = ILI9341_BLACK;


ParamEntry paramTable[] = {
  { PARAM_CALIBRATION, ""},
};

Params eeparam( paramTable, sizeof(paramTable)/sizeof(paramTable[0]));

void drawCursor(int16_t x, int16_t y, uint16_t color)
{
  int16_t r = 40;
  uint16_t w = 10;
  int16_t xx = x < r ? 0 : x - r;
  int16_t yy = y < r ? 0 : y - r;

  tft.fillRect(x - w/2, yy, w, r * 2, color);
  tft.fillRect(xx, y - w/2, r * 2, w, color);
}

void setup()
{
  bool r;
  char buf[80];
  
  Serial.begin(115200);
  Serial.println("\n\nReset:");
  tft.begin();
  ts.begin();

  tft.setTextSize(2);
  tft.setRotation(3);
  eeparam.load();

  if(ts.touched() || !ts.loadCalibrationData(&eeparam)){
    do {
      r = ts.calibrate(&tft, c1, bg);
    
      tft.fillScreen(bg);
      tft.setTextColor(c2);
      tft.setCursor(0,0);
      tft.print("calibration ");
      tft.println( r ? "succeeded." : "failed.");
      delay(1000);
    } while(!r);
    ts.saveCalibrationData(&eeparam);
    if(eeparam.changed())
      eeparam.save();
  }

  tft.fillScreen(bg);
  tft.setCursor(0,0);
  tft.setTextSize(3);
  tft.println("Touch screen");
  while(!ts.touched()) delay(100);
  
  tft.fillScreen(bg);
  tft.setTextColor(c1, bg);
}

void loop()
{
  static bool v0 = false;
  static int16_t x0, y0;
  int16_t x, y, d = 1;
  
  
  if(ts.getPoint(&x , &y)){
    if(!v0) {
      drawCursor(x, y, c3);
      x0 = x;
      y0 = y;
    }
    else if(x < x0 - d || x > x0 + d || y < y0 - d || y > y0 + d){
      drawCursor(x0, y0, bg);
      drawCursor(x, y, c3);
      x0 = x;
      y0 = y;
    }
    v0 = true;
    tft.setCursor(0,0);
    tft.printf("%3d,%3d", x, y);
  }
  else {
    drawCursor(x0, y0, bg);
    v0 = false;
  }

  delay(100);
}

