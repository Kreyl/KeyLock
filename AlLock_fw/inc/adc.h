/*
 * adc.h
 *
 *  Created on: 27.11.2011
 *      Author: Kreyl
 */

#ifndef ADC_H_
#define ADC_H_

#include <inttypes.h>
#include "kl_util.h"

#define BATTERY_EMPTY   2151    // 3.5V
#define BATTERY_HALF    2466    // 4V

#define ADC_AVERAGE_COUNT   16  // Number of times to measure

typedef enum {bsEmpty, bsHalf, bsFull} BatteryState_t;

class Adc_t {
private:
    uint16_t ADCValues[ADC_AVERAGE_COUNT];
protected:
    uint16_t IValue;
    uint32_t Timer;
    void Measure(void);
public:
    void Init(void);
};

class Battery_t : public Adc_t {
public:
    BatteryState_t State;
    void Task(void);
    ftVoid_Void EvtNewState;
};


extern Battery_t Battery;

#endif /* ADC_H_ */
