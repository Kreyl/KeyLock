/*
 * kl_gpio.cpp
 *
 *  Created on: 18.02.2012
 *      Author: kreyl
 */

#include "kl_lib.h"

void klGpio::SetupByMsk(GPIO_TypeDef *PGpioPort, uint16_t APinMask, GPIOMode_TypeDef AMode) {
    int IClock = 0;
    if (PGpioPort == GPIOA) IClock = RCC_APB2Periph_GPIOA;
    else if (PGpioPort == GPIOB) IClock = RCC_APB2Periph_GPIOB;
    // Clock
    if ((AMode == GPIO_Mode_AF_OD) || (AMode == GPIO_Mode_AF_PP)) IClock |= RCC_APB2Periph_AFIO;
    RCC_APB2PeriphClockCmd(IClock, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = APinMask;
    GPIO_InitStructure.GPIO_Mode = AMode;
    GPIO_Init(PGpioPort, &GPIO_InitStructure);
}

void klPin_t::Init(GPIO_TypeDef *PGpioPort, uint16_t APinNumber, GPIOMode_TypeDef AMode) {
    IPinNumber = APinNumber;
    IPinMask = (uint16_t)(1<<APinNumber);
    IGPIO = PGpioPort;
    klGpio::SetupByN(PGpioPort, APinNumber, AMode);
}

// ======== IRQ pin =========
void klPinIrq_t::IrqSetup(EXTITrigger_TypeDef ATriggerType) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    // Get IRQ channel
    if      ((IPinNumber >= 0)  && (IPinNumber <= 4))   IChannel = IPinNumber;
    else if ((IPinNumber >= 5)  && (IPinNumber <= 9))   IChannel = EXTI9_5_IRQn;
    else if ((IPinNumber >= 10) && (IPinNumber <= 15))  IChannel = EXTI15_10_IRQn;
    // EXTI line config
    if (IGPIO == GPIOA) GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, IPinNumber);
    else if (IGPIO == GPIOB) GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, IPinNumber);
    // Configure EXTI3 line
    EXTI_InitTypeDef   EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = IPinMask;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = ATriggerType;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

void klPinIrq_t::IrqEnable() {
    // Enable and set EXTI Interrupt
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = IChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void klPinIrq_t::IrqDisable() {
    NVIC_InitTypeDef   NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// ================================= Timer =====================================
void klTimer_t::Init(TIM_TypeDef* PTimer, uint16_t ATopValue, uint32_t AFreqHz) {
    ITimer = PTimer;
    // Clock
    if      (ITimer == TIM1) RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    else if (ITimer == TIM2) RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    else if (ITimer == TIM3) RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    else if (ITimer == TIM4) RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    // Timebase
    uint16_t IPrescaler = 0;
    if (AFreqHz != TIM_FREQ_MAX) {
        IPrescaler = SystemCoreClock / (ATopValue * AFreqHz);
        if (IPrescaler != 0) IPrescaler--;   // do not decrease in case of high freq
    }
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = ATopValue;
    TIM_TimeBaseStructure.TIM_Prescaler = IPrescaler;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(PTimer, &TIM_TimeBaseStructure);
}

void klPwmChannel_t::Init(TIM_TypeDef* PTimer, uint16_t ATopValue, uint32_t AFreqHz, uint8_t ANumber, uint16_t APolarity) {
    klTimer_t::Init(PTimer, ATopValue, AFreqHz);
    INumber = ANumber;

    TIM_OCInitTypeDef  TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = APolarity;
    switch (ANumber) {
        case 1:
            TIM_OC1Init(PTimer, &TIM_OCInitStructure);
            TIM_OC1PreloadConfig(PTimer, TIM_OCPreload_Enable);
            break;
        case 2:
            TIM_OC2Init(PTimer, &TIM_OCInitStructure);
            TIM_OC2PreloadConfig(PTimer, TIM_OCPreload_Enable);
            break;
        case 3:
            TIM_OC3Init(PTimer, &TIM_OCInitStructure);
            TIM_OC3PreloadConfig(PTimer, TIM_OCPreload_Enable);
            break;
        case 4:
            TIM_OC4Init(PTimer, &TIM_OCInitStructure);
            TIM_OC4PreloadConfig(PTimer, TIM_OCPreload_Enable);
            break;
        default: break;
    }
    TIM_ARRPreloadConfig(PTimer, ENABLE);   // Enable autoreload of preload
    klTimer_t::Enable();
}

void klPwmChannel_t::Enable() {
    switch (INumber) {
        case 1: ITimer->CCER |= TIM_CCER_CC1E; break;
        case 2: ITimer->CCER |= TIM_CCER_CC2E; break;
        case 3: ITimer->CCER |= TIM_CCER_CC3E; break;
        case 4: ITimer->CCER |= TIM_CCER_CC4E; break;
        default: break;
    }
}

void klPwmChannel_t::Disable() {
    switch (INumber) {
        case 1: ITimer->CCER &= ~TIM_CCER_CC1E; break;
        case 2: ITimer->CCER &= ~TIM_CCER_CC2E; break;
        case 3: ITimer->CCER &= ~TIM_CCER_CC3E; break;
        case 4: ITimer->CCER &= ~TIM_CCER_CC4E; break;
        default: break;
    }
}

void klPwmChannel_t::Set(uint16_t AValue) {
    switch (INumber) {
        case 1: ITimer->CCR1 = AValue; break;
        case 2: ITimer->CCR2 = AValue; break;
        case 3: ITimer->CCR3 = AValue; break;
        case 4: ITimer->CCR4 = AValue; break;
        default: break;
    }
}
