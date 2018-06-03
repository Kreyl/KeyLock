/*
 * klstring.h
 *
 *  Created on: 21.11.2011
 *      Author: Kreyl
 */

#ifndef KLSTRING_H_
#define KLSTRING_H_

#include <stdbool.h>
#include <inttypes.h>
#include "stm32f10x_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

// Gpio
void GpioPinConfig(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef AMode);

// Types
#define FORCE_INLINE inline __attribute__ ((__always_inline__))
#define PACKED __attribute__ ((__packed__))

// Common types
typedef enum {erOk, erTimeout, erError, erBusy} Error_t;

// Functional types
typedef void(*ftVoid_Void)(void);


// Prototypes
char *UintToStr(uint32_t AValue, char* S);
void UART_Init(void);
void klPrintf(const char *S, ...);



bool HexToUint(char *S, uint8_t AMaxLength, uint32_t *AOutput);

#ifdef __cplusplus
}
#endif

#endif /* KLSTRING_H_ */
