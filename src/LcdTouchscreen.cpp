#include <Arduino.h>
#include "LcdTouchscreen.h"

LcdTouchscreen::LcdTouchscreen(uint8_t cs_pin) : m_ts(cs_pin)
{
}

bool LcdTouchscreen::begin()
{
  return m_ts.begin();
}

bool LcdTouchscreen::getPoint(int16_t *pX, int16_t *pY, uint16_t *pZ)
{
  int16_t x,y;

  if(m_ts.touched()){
    TS_Point pt = m_ts.getPoint();

    x = m_a[0] + (((int32_t)m_b[0][0]*pt.x+(int32_t)m_b[0][1]*pt.y)>>12);
    y = m_a[1] + (((int32_t)m_b[1][0]*pt.x+(int32_t)m_b[1][1]*pt.y)>>12);

    if(pX) *pX = x;
    if(pY) *pY = y;
    if(pZ) *pZ = pt.z;
    return true;
  }
  return false;
}

bool LcdTouchscreen::getRawPoint(int16_t *pX, int16_t *pY, uint16_t *pZ)
{
  if(m_ts.touched()){
    TS_Point pt = m_ts.getPoint();
    if(pX) *pX = pt.x;
    if(pY) *pY = pt.y;
    if(pZ) *pZ = pt.z;
    return true;
  }
  return false;
}

static void calcLinearRegression(const CalibData *aData, uint8_t nData,
				 int ix, int iy,
				 int16_t *pA,	// y = A + B * x;
				 int16_t *pB,
				 int16_t *pC2)	
{
  int i,n = nData;
  double x,y,sx,sy,sxx,syy,sxy;
  double xm,ym;
  double a,b,c2;
  
  sx = sy = sxx = syy = sxy = 0;
  for(i=0;i<n;i++){
    x = aData[i].tp[ix];
    y = aData[i].lcd[iy];
    sx  += x;
    sy  += y;
    sxx += x * x;
    syy += y * y;
    sxy += x * y;
  }
  xm = sx / n;
  ym = sy / n;
  sxx = sxx / n - xm*xm;
  syy = syy / n - ym*ym;
  sxy = sxy / n - xm*ym;
  
  b = sxy / sxx;
  a = (sy - sxy * sx / sxx ) / n;
  c2 = sxy * sxy / sxx / syy;

  *pA = (int16_t)a;
  *pB = (int16_t)(b * (1 << 12));
  *pC2 = (int16_t)(c2 * (1 << 8));
}

#define LinearityThreshold	250

bool LcdTouchscreen::setCalibration(const CalibData *aData, uint16_t nData)
{
  int16_t a[2][2],b[2][2],c[2][2];

  if(nData < 3) return false;
  
  for(int i=0; i<2; i++){
    for(int j=0; j<2; j++){
      calcLinearRegression(aData, nData, i, j,
			   &a[i][j], &b[i][j], &c[i][j]);
    }
  }

  if(c[0][0] > LinearityThreshold && c[1][1] > LinearityThreshold){
    m_a[0] = a[0][0];
    m_a[1] = a[1][1];
    m_b[0][0] = b[0][0];
    m_b[0][1] = 0;
    m_b[1][0] = 0;
    m_b[1][1] = b[1][1];
  }
  else if(c[0][1] > LinearityThreshold && c[1][0] > LinearityThreshold){
    m_a[0] = a[1][0];
    m_a[1] = a[0][1];
    m_b[0][0] = 0;
    m_b[0][1] = b[1][0];
    m_b[1][0] = b[0][1];
    m_b[1][1] = 0;
  }
  else return false;

  return true;
}

bool LcdTouchscreen::getCalibrationData(uint16_t xs, uint16_t ys, CalibData *data)
{
  int r = 10;
  int16_t xt, yt;
  
  m_dpy->fillScreen(m_bgColor);
  m_dpy->drawLine(xs-r, ys, xs+r, ys, m_fgColor);
  m_dpy->drawLine(xs, ys-r, xs, ys+r, m_fgColor);

  while(!getRawPoint(&xt, &yt)){
    delay(50);
  }
  data->lcd[0] = xs;
  data->lcd[1] = ys;
  data->tp[0]  = xt;
  data->tp[1]  = yt;
  return true;
}


bool LcdTouchscreen::calibrate(Adafruit_GFX *dpy, uint16_t fgColor,
			       uint16_t bgColor)
{
  uint16_t w,h;
  uint16_t m = 30;
  CalibData calibData[4];
  
  m_dpy = dpy;
  m_fgColor = fgColor;
  m_bgColor = bgColor;
  w = m_dpy->width();
  h = m_dpy->height();

  while(touched())
    delay(50);

  return
    getCalibrationData( m    ,     m, &calibData[0]) &&
    getCalibrationData( w - m,     m, &calibData[1]) &&
    getCalibrationData( w - m, h - m, &calibData[2]) &&
    getCalibrationData( m    , h - m, &calibData[3]) &&
    setCalibration(calibData, 4);
}

void LcdTouchscreen::getCalibrationString(char *buf, size_t buf_len)
{
  snprintf(buf,buf_len,"%d,%d,%d,%d,%d,%d",
	   m_a[0],m_a[1],m_b[0][0],m_b[0][1],m_b[1][0],m_b[1][1]);
}

bool LcdTouchscreen::saveCalibrationData(Params *params)
{
  char buf[80];

  getCalibrationString(buf, sizeof buf);
  params->set(PARAM_CALIBRATION, buf);
  return true;
}

bool LcdTouchscreen::loadCalibrationData(Params *params)
{
  const char *data;

  if(params->get(PARAM_CALIBRATION, &data) && data[0]){
    const char *p = data;
    char *p0;
    int i,d[6];

    p = data;
    for(i=0;i<6; i++){
      d[i] = strtol(p,&p0, 0);
      if(p == p0) return false;
      if(*p0 != ',') break;
      p = p0+1;
    }
    if(i!=5) return false;
    
    m_a[0] = d[0];
    m_a[1] = d[1];
    m_b[0][0] = d[2];
    m_b[0][1] = d[3];
    m_b[1][0] = d[4];
    m_b[1][1] = d[5];
    return true;
  }
  return false;
}
