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

#define ADC_AVERAGE_COUNT   16  // Number of times to measure

class Adc_t {
private:
    uint16_t ADCValues[ADC_AVERAGE_COUNT];
protected:
    uint16_t IValue;
public:
    void Init(void);
    uint32_t Measure(void);
};

extern Adc_t Adc;

#endif /* ADC_H_ */
