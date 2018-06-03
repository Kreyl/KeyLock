/*
 * File:   time_utils.h
 * Author: Laurelindo
 *
 * Created on 2009 г., 0:40
 */

#ifndef _TIME_UTILS_H
#define	_TIME_UTILS_H

//#include <inttypes.h>
//#include <stdbool.h>
//#include
#include "stm32f10x.h"

class Delay_t {
public:
    uint32_t TickCounter;
    void Init(void);
    // Simple loop-based delays, no init needed
    void Loop (volatile uint32_t ACounter);
    void ms (uint32_t Ams);
    // Interrupt-driven delays
    bool Elapsed(uint32_t *AVar, const uint32_t ADelay);
    void Reset  (uint32_t *AVar) { *AVar = TickCounter; }
    void Bypass (uint32_t *AVar, const uint32_t ADelay) { *AVar = TickCounter - ADelay; }

    void IncreaseTickCounter(void) { TickCounter++; }
};

extern Delay_t Delay;

// Declare Timer IRQ. Use externC to make it visible from asm file.
extern "C" {
void TIM2_IRQHandler(void);
}


#endif	/* _TIME_UTILS_H */

