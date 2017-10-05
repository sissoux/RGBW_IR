#include "RGBWLamp.h"
#include <ArduinoJson.h>
#include <IRremote.h>
#include "IRdefine.h"


elapsedMillis RefreshOutputTimer = 0;
#define OUTPUT_REFRESH_RATE 7
elapsedMillis IRRepeatTimeout = 0;
#define IR_REPEAT_TIMEOUT 200
elapsedMillis TempMeasTimer = 0;
#define TEMP_MEAS_RATE 60000
#define TEMP_DERATE_THRESHOLD 200
#define OVERTEMP_THRESHOLD 138
elapsedMillis AutoTimer = 0;
uint16_t AutoDelay = 500;

RGBWLamp Lamp;
bool AutoFade = true;
bool AutoJump = false;
bool HiSpeed = false;

#define MIN_HISPEED_DELAY 1000
#define MAX_HISPEED_DELAY 10000
#define MIN_LOSPEED_DELAY 10000
#define MAX_LOSPEED_DELAY 30000


DynamicJsonBuffer jsonBuffer;
char input[100];


void setup()
{
  Lamp.begin();
  Serial.begin(115200);
  IR.enableIRIn();
}

void loop()
{
  taskManager();
}


void taskManager()
{
  IR_Management();
  serialParse();

  if (RefreshOutputTimer >= OUTPUT_REFRESH_RATE)
  {
    RefreshOutputTimer = 0;
    Lamp.refreshState();
  }

  if (TempMeasTimer >= TEMP_MEAS_RATE)
  {
    TempMeasTimer = 0;
    uint16_t val = analogRead(A0);
    if ( val <= (OVERTEMP_THRESHOLD + Lamp.overTemp * 50) ) //Check if we are overtemp with hysteresis
    {
      Lamp.overTemp = 1;
    }
    else if ( val <= TEMP_DERATE_THRESHOLD)
    {
      Lamp.overTemp = 0;
      Lamp.decreaseBrightness();
    }
    else
    {
      Lamp.overTemp = 0;
    }
  }

  if (AutoTimer >= AutoDelay && (AutoFade || AutoJump))
  {
    AutoTimer = 0;
    if (HiSpeed)
    {
      AutoDelay = random(MIN_HISPEED_DELAY, MAX_HISPEED_DELAY);
    }
    else
    {
      AutoDelay = random(MIN_LOSPEED_DELAY, MAX_LOSPEED_DELAY);
    }
    double NewHue = (double)(random(0, 36000) / 100.0);
    double NewSat = (double)(random(10, 1000) / 1000.0);
    double NewVal = (double)(random(10, 1000) / 1000.0);

    if (AutoFade) Lamp.fade(NewHue, NewSat, NewVal, (AutoDelay) / (random(2, 3)));
    else if (AutoJump) Lamp.setColor(NewHue, NewSat, NewVal);
  }
}

void serialParse()
{
  if (Serial.available()) {
    Serial.println("Serial received.");
    unsigned long time = micros();
    char lastChar = '\0';
    int i = 0;

    while (lastChar != '\r') {
      if (Serial.available()) {
        lastChar = Serial.read();
        input[i] = lastChar;
        i++;
      }
    }

    JsonObject& root = jsonBuffer.parseObject(input);
    const char* method = root["method"];
    Serial.println(method);
    if (strcmp(method, "setRGBW") == 0) //{method:setRGBW,R:65535,G:0,B:65535,W:0}
    {
      Serial.println("set to RGBW value");
      Lamp.outputWrite(root["R"], root["G"], root["B"], root["W"]);
    }
    else if (strcmp(method, "setHSV") == 0)//{method:setHSV,H:120,S:0.1,V:1}
    {
      Serial.println("set to HSV value");
      Lamp.setColor(root["H"], root["S"], root["V"]);
    }
    else if (strcmp(method, "FadeHSV") == 0)//{method:FadeHSV,H:60,S:1,V:1,D:1000}
    {
      Serial.println("Fade to HSV value");
      Lamp.fade(root["H"], root["S"], root["V"], root["D"]);
    }
    else
    {
      Serial.println("Undefined JSON");
    }
  }
}


void IR_Management()
{
  ircmd CurrentCommand = getIRCmd();

  if (CurrentCommand == NO_CMD)
  {
    return;
  }

  if ( CurrentCommand == RPT)
  {
    if (IR_rptTimer >= IR_REPEAT_TIMEOUT)
    {
      IR_rptTimer = 0;
      CurrentCommand = LastIRCmd;
    }
  }

  if ( CurrentCommand < Rp)   //First commands in the list are for static colors
  {
    Lamp.setColor(StaticColorHue[CurrentCommand], StaticColorSat[CurrentCommand], 1);
    LastIRCmd = NO_CMD;
    AutoFade = false;
    AutoJump = false;
    Lamp.stopFx();
  }
  else
  {
    switch (CurrentCommand)
    {
      case ip:
        Lamp.increaseBrightness();
        IRRepeatTimeout = 0;
        LastIRCmd = ip;
        break;
      case im:
        Lamp.decreaseBrightness();
        IRRepeatTimeout = 0;
        LastIRCmd = im;
        break;
      case Auto:
        AutoFade = true;
        AutoJump = false;
        LastIRCmd = NO_CMD;
        break;
      case Fadea:
        AutoFade = true;
        AutoJump = false;
        LastIRCmd = NO_CMD;
        break;
      case Jumpa:
        AutoFade = false;
        AutoJump = true;
        LastIRCmd = NO_CMD;
        break;
      case Fadeb:
        AutoFade = true;
        AutoJump = false;
        LastIRCmd = NO_CMD;
        break;
      case Jumpb:
        AutoFade = false;
        AutoJump = true;
        LastIRCmd = NO_CMD;
        break;

      case Spdp:
        HiSpeed = true;
        LastIRCmd = NO_CMD;
        break;
      case Spdm:
        HiSpeed = false;
        LastIRCmd = NO_CMD;
        break;

      case Pwr:
        Lamp.toogle();
        AutoFade = false;
        AutoJump = false;
        break;
      default: break;
    }
  }
}

