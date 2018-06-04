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

enum LedMode_t {lmSolid, lmBlink};

class Led_t {
private:
    uint32_t Timer, BlinkDelay;
public:
    LedMode_t Mode;
    Color_t Color, *PWM;
    bool IsOn, HasChanged;
    void Task();
    void Solid(Color_t AColor);
    void Blink(Color_t AColor, uint32_t ADelay);
};

// PCA registers
struct LedsPkt_t {
    uint8_t ControlReg;
    // Registers
    uint8_t Mode1;
    uint8_t Mode2;
    uint8_t PWM[16];    // PWM channels
    uint8_t GrpPWM;
    uint8_t GrpFreq;
    uint8_t LEDOut[4];
} __attribute__ ((packed));
#define LEDS_PKT_SIZE   sizeof(LedsPkt_t)

#define LED_COUNT   5
class Leds_t {
private:
    LedsPkt_t IPkt;
//    I2C_Cmd_t i2cCmd;
//    void SendCmd(void) { i2cMgr.AddCmd(i2cCmd); }
    void SetPWM(uint8_t AChannel, uint8_t AValue);
public:
    // General methods
    void Init();
    void Task();
    void OutputEnable () { PinSetLo(LED_OE_PIN); }
    void OutputDisable() { PinSetHi(LED_OE_PIN);   }
    void SetColor(uint8_t Indx, Color_t Clr);
    // Light effects
    Led_t Led[LED_COUNT];
};

extern Leds_t Leds;
