#include "RGBWLamp.h"
#include <ArduinoJson.h>
#include <IRremote.h>
#include "IRdefine.h"


elapsedMillis RefreshOutputTimer = 0;
#define OUTPUT_REFRESH_RATE 7
elapsedMillis IRRepeatTimeout = 0;
#define IR_REPEAT_TIMEOUT 200
elapsedMillis TempMeasTimer = 0;
#define TEMP_MEAS_RATE 3000
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

uint16_t Temp_Lookup[118][2] = {{1024, 8}, {1009, 9}, {995, 10}, {981, 11}, {966, 12}, {951, 13}, {937, 14}, {922, 15}, {907, 16}, {892, 17}, {878, 18}, {863, 19}, {848, 20}, {833, 21}, {819, 22}, {804, 23}, {790, 24}, {775, 25}, {760, 26}, {746, 27}, {732, 28}, {718, 29}, {704, 30}, {690, 31}, {676, 32}, {662, 33}, {649, 34}, {636, 35}, {622, 36}, {609, 37}, {596, 38}, {584, 39}, {571, 40}, {559, 41}, {547, 42}, {534, 43}, {523, 44}, {511, 45}, {499, 46}, {488, 47}, {477, 48}, {466, 49}, {455, 50}, {445, 51}, {435, 52}, {424, 53}, {414, 54}, {405, 55}, {395, 56}, {386, 57}, {377, 58}, {368, 59}, {359, 60}, {351, 61}, {342, 62}, {334, 63}, {326, 64}, {318, 65}, {311, 66}, {304, 67}, {296, 68}, {289, 69}, {282, 70}, {276, 71}, {269, 72}, {263, 73}, {256, 74}, {250, 75}, {244, 76}, {238, 77}, {233, 78}, {227, 79}, {222, 80}, {216, 81}, {211, 82}, {206, 83}, {201, 84}, {197, 85}, {192, 86}, {187, 87}, {183, 88}, {179, 89}, {174, 90}, {170, 91}, {166, 92}, {162, 93}, {159, 94}, {155, 95}, {151, 96}, {148, 97}, {144, 98}, {141, 99}, {138, 100}, {134, 101}, {131, 102}, {128, 103}, {125, 104}, {122, 105}, {120, 106}, {117, 107}, {114, 108}, {112, 109}, {109, 110}, {107, 111}, {104, 112}, {102, 113}, {100, 114}, {98, 115}, {95, 116}, {93, 117}, {91, 118}, {89, 119}, {87, 120}, {85, 121}, {83, 122}, {82, 123}, {80, 124}, {78, 125}};
#define NB_OF_VALUE 118

uint8_t getTemp(uint16_t val)
{
  for (int i = 0; i < NB_OF_VALUE; i++)
  {
    if (val <= Temp_Lookup[i][0] && val > Temp_Lookup[i + 1][0]) return Temp_Lookup[i][1];
  }
}


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
    /*if (StripCommander.RunningFX)  //Update output only if necessary, RefreshOutputTimer is reset only if frame is displayed ==> As soon as one frame il
      {
      StripCommander.StateChanged = false;
      FastLED.show();
      }
      else if (!LastFrameShowed)  //Show one more frame after RunningFX has been reset to be sure last effect iteration has been showed.
      {
      LastFrameShowed = true;
      FastLED.show();
      }
      StripCommander.dynamicStateUpdate();*/

  }
  if (TempMeasTimer >= TEMP_MEAS_RATE)
  {
    TempMeasTimer = 0;
    uint16_t val = analogRead(A0);
    Serial.print("ADC Value: ");
    Serial.print(val);
    Serial.print(", Temp: ");
    Serial.println(getTemp(val));
  }
  if (AutoTimer >= AutoDelay)
  {
    AutoTimer = 0;
    if (AutoFade || AutoJump)
    {
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

  if ( CurrentCommand < Rp)
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
        AutoTimer = 30000;
        AutoFade = true;
        AutoJump = false;
        LastIRCmd = NO_CMD;
        break;
      case Fadea:
        AutoTimer = 30000;
        AutoFade = true;
        AutoJump = false;
        LastIRCmd = NO_CMD;
        break;
      case Jumpa:
        AutoTimer = 30000;
        AutoFade = false;
        AutoJump = true;
        LastIRCmd = NO_CMD;
        break;
      case Fadeb:
        AutoTimer = 30000;
        AutoFade = true;
        AutoJump = false;
        LastIRCmd = NO_CMD;
        break;
      case Jumpb:
        AutoTimer = 30000;
        AutoFade = false;
        AutoJump = true;
        LastIRCmd = NO_CMD;
        break;

      case Spdp:
        AutoTimer = 30000;
        HiSpeed = true;
        LastIRCmd = NO_CMD;
        break;
      case Spdm:
        AutoTimer = 30000;
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


