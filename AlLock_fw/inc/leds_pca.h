/*
 * File:   leds_pca.h
 * Author: Kreyl
 *
 * Created on May 31, 2011, 8:44 PM
 */

#ifndef LEDS_PCA_H
#define	LEDS_PCA_H

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "i2c_mgr.h"
#include "delay_util.h"
#include "color.h"

#define LED_I2C_ADDR    0x1B

enum LedMode_t {lmSolid, lmBlink};

class Led_t {
private:
    uint32_t Timer, BlinkDelay;
public:
    LedMode_t Mode;
    Color_t Color, *PWM;
    bool IsOn, HasChanged;
    void Task(void);
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
    I2C_Cmd_t i2cCmd;
    void SendCmd(void) { i2cMgr.AddCmd(i2cCmd); }
public:
    // General methods
    void Init(void);
    void Task(void);
    void OutputEnable (void) { GPIO_ResetBits(GPIOA, GPIO_Pin_12); }
    void OutputDisable(void) { GPIO_SetBits(GPIOA, GPIO_Pin_12);   }
    // Light effects
    Led_t Led[LED_COUNT];
    void SetPWM(uint8_t AChannel, uint8_t AValue);
};

extern Leds_t Leds;

#endif	/* LEDS_PCA_H */

