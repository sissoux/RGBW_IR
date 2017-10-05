#ifndef IRDEFINE_H
#define IRDEFINE_H

#include "Arduino.h"
#include <IRremote.h>

#define IR_PIN 9
#define NB_IR_CMD 44
#define REPEAT_TIMEOUT 250
elapsedMillis IR_rptTimer = 0;

uint16_t IRcmdCommands[NB_IR_CMD] = {0x1AE5, 0x9A65, 0xA25D, 0x22DD, 0x2AD5, 0xAA55, 0x926D, 0x12ED, 0x0AF5, 0x8A75, 0xB24D, 0x32CD, 0x38C7, 0xB847, 0x7887, 0xF807, 0x18E7, 0x9867, 0x58A7, 0xD827, 0x28D7, 0x08F7, 0xA857, 0x8877, 0x6897, 0x48B7, 0x02FD, 0x3AC5, 0xBA45, 0x827D, 0xE817, 0xC837, 0xF00F, 0xD02F, 0x30CF, 0xB04F, 0x708F, 0x10EF, 0x906F, 0x50AF, 0x609F, 0xE01F, 0x20DF, 0xA05F};

typedef enum {
  R, G, B, W, Ca, Cb, Cc, Cd, Ce, Cf, Cg, Ch, Ci, Cj, Ck, Cl, Cm, Cn, Co, Cp, Rp, Rm, Gp, Gm, Bp, Bm, Pwr, ip, im, Play, Spdp, Spdm, Auto, Flash, DIYa, DIYb, DIYc, DIYd, DIYe, DIYf, Fadea, Fadeb, Jumpa, Jumpb, NO_CMD, RPT
}ircmd;

ircmd StaticColorTable[20] = {R, G, B, W, Ca, Cb, Cc, Cd, Ce, Cd, Cf, Ch, Ci, Cj, Ck, Cl, Cm, Cn, Co, Cp};

double StaticColorHue[20] = {0,  120,  240,  0,  16,  112,  176,  224,  32,  128,  192,  224,  48,  134,  199,  135,  64,  144,  210,  139};
double StaticColorSat[20] = {1,  1,  1,  0,  1,  1,  1,  0.45,  1,  1,  1,  0.7,  1,  1,  1,  0.7,  1,  1,  1,  0.7};


ircmd MyCMD = NO_CMD;
IRrecv IR(IR_PIN);
decode_results RawIRCmd;
ircmd LastIRCmd = NO_CMD;



ircmd getIRCmd()
{
  if ( IR.decode(&RawIRCmd))
  {
    if ((RawIRCmd.value == 0xFFFFFFFF))
    {
      IR.resume();
      return RPT;
    }
    else
    {
      for (int i = 0 ; i < NB_IR_CMD ; i++)
      {
        //Serial.println(RawIRCmd.value & 0x00FFFF, HEX);
        if ((RawIRCmd.value & 0x00FFFF) == IRcmdCommands[i])
        {
          IR.resume();
          return (ircmd)i;
        }
      }
      IR.resume();
    }
  }
  return NO_CMD;
}



#endif


