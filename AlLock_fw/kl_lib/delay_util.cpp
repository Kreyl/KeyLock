#include "delay_util.h"
#include "stm32f10x_tim.h"
#include "misc.h"
#include "stm32f10x_rcc.h"

Delay_t Delay;
// =============================== Implementation ==============================

void Delay_t::Init(void) {
    // Interrupt config
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // TIM2 clock enable
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    // Time base configuration: 1 ms == 1000 Hz = FCLK / (100 * (FCLK/100 000))
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // Up-counter needed, nothing special
    TIM_TimeBaseStructure.TIM_ClockDivision = 0; // Dead-time divisor, not needed here
    TIM_TimeBaseStructure.TIM_Period = 100; // Auto-reload value
    TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t) (SystemCoreClock / 100000) - 1; // Input clock divisor
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    // Enable timer
    TIM_Cmd(TIM2, ENABLE);
    // Clear Timer update pending flag
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    // Interrupts config
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

bool Delay_t::Elapsed(uint32_t *AVar, const uint32_t ADelay) {
    if ((TickCounter - *AVar) >= ADelay) {
        *AVar = TickCounter; // Reset delay
        return true;
    } else return false;
}

void Delay_t::Loop(volatile uint32_t ACounter) {
    for (; ACounter != 0; ACounter--);
}

void Delay_t::ms(uint32_t Ams) {
    uint32_t __ticks = (SystemCoreClock / 10000) * Ams;
    Loop(__ticks);
}

// ================================ Interrupts =================================
// Delay counter
void TIM2_IRQHandler(void) {
    // Clear TIM2 update interrupt
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    Delay.IncreaseTickCounter();
}
