/*
 * File:   leds_pca.h
 * Author: Kreyl
 *
 * Created on May 31, 2011, 8:44 PM
 */

#pragma once

#include "color.h"
#include "kl_lib.h"

#define LED_I2C_ADDR    0x1B

class Leds_t {
private:
    void SetPWM(uint8_t AChannel, uint8_t AValue);
public:
    void Init();
    void Task();
    void OutputEnable () { PinSetLo(LED_OE_PIN); }
    void OutputDisable() { PinSetHi(LED_OE_PIN);   }
    void SetColor(uint8_t Indx, Color_t Clr);
};

extern Leds_t Leds;
