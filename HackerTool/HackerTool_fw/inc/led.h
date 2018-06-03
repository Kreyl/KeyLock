/*
 * led.h
 *
 *  Created on: 03.11.2011
 *      Author: g.kruglov
 */

#ifndef LED_H_
#define LED_H_

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "delay_util.h"

#define LED_BLINK_DELAY 144

class Led_t {
private:
    uint32_t Timer;
    GPIO_TypeDef* IGPIO;
    uint16_t IPin;
    bool IsOn, IsInsideBlink;
public:
    void On(void)      { GPIO_SetBits(IGPIO, IPin); IsOn = true;}
    void Off(void)     { GPIO_ResetBits(IGPIO, IPin); IsOn = false; }
    void Disable(void) { Off(); IsInsideBlink = false; }
    void Init(GPIO_TypeDef* AGPIO, uint16_t APin);
    void Blink(void);
    void Task(void);
    void Toggle(void)  { IGPIO->ODR ^= IPin; IsOn = !IsOn; }
};

extern Led_t Led1, Led2;

#endif /* LED_H_ */
