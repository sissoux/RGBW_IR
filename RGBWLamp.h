#ifndef RGBWLamp_H
#define RGBWLamp_H

#include "Arduino.h"
#include "FastLED.h"

#define PIST 1.0471975512
#define DPIST 2.09439510239
#define QPIST 4.18879020479
#define PI 3.14159265359

#define R_VALUE_REG FTM0_C1V
#define G_VALUE_REG FTM0_C0V
#define B_VALUE_REG FTM0_C6V
#define W_VALUE_REG FTM0_C5V

#define R_PIN 23
#define G_PIN 22
#define B_PIN 20
#define W_PIN 21

#define RUNNING 2
#define TRIGGER 1
#define STBY 0
#define BRIGHTNESS_CHANGE 3

#define INTENSITY_INCREMENT 0.05



class RGBWLamp
{
  public:
    RGBWLamp();
    void begin();
    void setColor(double h, double s, double v);
    void setColorCIE1931(double x, double y, double Y);
    //void setColor(const struct CRGB& color);
    void outputWrite(uint16_t R, uint16_t G, uint16_t B, uint16_t W);
    void refreshState();
    void fade(double h, double s, double v, uint16_t delay);
    void increaseBrightness();
    void decreaseBrightness();
    void stopFx();
    void toogle();


  private :
    double Brightness = 1.0;
    elapsedMillis FxTimer = 0;
    double h, s = 1, v = 0;
    double ah, bh, as, bs, av, bv;
    uint8_t FxState = STBY;
    uint16_t FxDuration = 0;
    uint16_t r, g, b, w;
    void timerSetup();
    void outputWrite();
};

#endif

