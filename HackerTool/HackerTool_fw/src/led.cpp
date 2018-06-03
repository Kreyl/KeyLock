/*
 * led.cpp
 *
 *  Created on: 03.11.2011
 *      Author: g.kruglov
 */

#include "led.h"
#include "stm32f10x_rcc.h"

Led_t Led1, Led2;

// =============================== Led_t =======================================
void Led_t::Init(GPIO_TypeDef* AGPIO, uint16_t APin) {
    IGPIO = AGPIO;
    IPin = APin;
    IsOn = false;
    IsInsideBlink = false;
    // Clock
    if (AGPIO == GPIOA) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    else if (AGPIO == GPIOB) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    else if (AGPIO == GPIOC) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    // Pin
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin  = APin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(AGPIO, &GPIO_InitStructure);
    Off();
}

void Led_t::Blink(void) {
    IsInsideBlink = true;
    On();
    Delay.Reset(&Timer);
}

void Led_t::Task() {
    if (!IsInsideBlink) return;
    if (Delay.Elapsed(&Timer, LED_BLINK_DELAY)) {
        IsInsideBlink = false;
        if (IsOn) Off();
        else On();
    }
}
