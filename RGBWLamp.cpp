#include "RGBWLamp.h"

RGBWLamp::RGBWLamp()
{}


void RGBWLamp::begin()
{
  pinMode(R_PIN, OUTPUT);
  analogWrite(R_PIN, 1);
  pinMode(G_PIN, OUTPUT);
  analogWrite(G_PIN, 1);
  pinMode(B_PIN, OUTPUT);
  analogWrite(B_PIN, 1);
  pinMode(W_PIN, OUTPUT);
  analogWrite(W_PIN, 1);

  FTM0_SC = 0b10001001;
  FTM0_C0SC = 0b00101000;
  FTM0_CNTIN = 0;
  FTM0_C0V = 0x00;
  FTM0_C1V = 0x00;
  FTM0_C5V = 0x00;
  FTM0_C6V = 0x00;
  FTM0_MOD = 0xFFFF;
}

void RGBWLamp::refreshState()
{
  if (FxState == STBY) return;
  if (FxState == TRIGGER)
  {
    uint16_t Time = FxTimer;
    if (FxTimer >= FxDuration)
    {
      FxState = STBY;
      FxTimer = FxDuration;
    }
    this->h = ah * (double)FxTimer + bh;
    this->s = as * (double)FxTimer + bs;
    this->v = av * (double)FxTimer + bv;
    Serial.println(h);
    setColor(this->h, this->s, this->v);
  }
  else if (FxState == BRIGHTNESS_CHANGE)
  {
    setColor(this->h, this->s, this->v);
  }
}
void RGBWLamp::stopFx()
{
  FxState = STBY;
}
void RGBWLamp::toogle()
{
  FxState = STBY;
  setColor(0, 1, 0);
}

void RGBWLamp::fade(double TargetHue, double TargetSat, double TargetVal, uint16_t duration)
{
  if ( FxState != STBY) return;
  ah = (TargetHue - this->h) / ((double)duration);
  bh = (this->h);
  as = (TargetSat - this->s) / ((double)duration);
  bs = (this->s);
  av = (TargetVal - this->v) / ((double)duration);
  bv = (this->v);
  FxState = TRIGGER;
  FxDuration = duration;
  FxTimer = 0;
}

void RGBWLamp::setColor(double hue, double saturation, double value)
{
  if (hue < 360 && hue >= 0 && saturation <= 1 && saturation >= 0 && value <= 1 && value >= 0)
  {
    double cos_h, cos_1047_h, R, G, B, W;
    double H = PI * hue / 180.0;

    this->h = hue;
    this->s = saturation;
    this->v = value;
    value = value * Brightness;

    if (H < DPIST) //2*PI/3
    {
      cos_h = cos(H);
      cos_1047_h = cos(PIST - H);
      R = saturation * value / 3 * (1 + cos_h / cos_1047_h);
      G = saturation * value / 3 * (1 + (1 - cos_h / cos_1047_h));
      B = 0;
      W = (1 - saturation) * value;
    }
    else if (H < QPIST)   // 4*PI/3
    {
      H = H - DPIST;
      cos_h = cos(H);
      cos_1047_h = cos(PIST - H);
      G = saturation * value / 3 * (1 + cos_h / cos_1047_h);
      B = saturation * value / 3 * (1 + (1 - cos_h / cos_1047_h));
      R = 0;
      W = (1 - saturation) * value;
    }
    else
    {
      H = H - QPIST;
      cos_h = cos(H);
      cos_1047_h = cos(PIST - H);
      B = saturation * value / 3 * (1 + cos_h / cos_1047_h);
      R = saturation * value / 3 * (1 + (1 - cos_h / cos_1047_h));
      G = 0;
      W = (1 - saturation) * value;
    }
    this->r = (uint16_t)(R * (double)0xFFFF);
    this->g = (uint16_t)(G * (double)0xFFFF);
    this->b = (uint16_t)(B * (double)0xFFFF);
    this->w = (uint16_t)(W * (double)0xFFFF);
    outputWrite();
  }
}


void RGBWLamp::setColorCIE1931(double x, double y, double Y)
{

}


void RGBWLamp::outputWrite(uint16_t R, uint16_t G, uint16_t B, uint16_t W)
{
  this->r = R;
  this->g = G;
  this->b = B;
  this->w = W;
  outputWrite();
}

void RGBWLamp::outputWrite()
{
  R_VALUE_REG = this->r;
  G_VALUE_REG = this->g;
  B_VALUE_REG = this->b;
  W_VALUE_REG = this->w;
}


void RGBWLamp::increaseBrightness()
{
  Brightness += INTENSITY_INCREMENT;
  if ( Brightness > 1) Brightness = 1;
  FxState = BRIGHTNESS_CHANGE;
}

void RGBWLamp::decreaseBrightness()
{
  Brightness -= INTENSITY_INCREMENT;
  if ( Brightness < 0) Brightness = 0;
  FxState = BRIGHTNESS_CHANGE;
}


