/*
 * File:   sensors.h
 * Author: Kreyl
 *
 * Created on June 19, 2011, 11:59 AM
 */

#ifndef SENSORS_H
#define	SENSORS_H

#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "delay_util.h"
#include "kl_util.h"

#define DETECTION_MAX_DELAY     11  // seconds

class Sns_t {
private:
    uint32_t Timer, TimerDetection;
    bool WasDetected;
    bool Detected(void) { return (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == Bit_SET); }
public:
    ftVoid_Void EvtDetected;
    void Init(void);
    void Task(void);
};


// =============================== Global ======================================
extern Sns_t Sensor;

#endif	/* SENSORS_H */

