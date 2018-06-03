/*
 * keys.h
 *
 *  Created on: 08.02.2012
 *      Author: kreyl
 */

#ifndef KEYS_H_
#define KEYS_H_

#include <inttypes.h>
#include "kl_util.h"
#include "delay_util.h"

// Timings
#define KEYS_RATE   45  // ms
// Keys
#define KEY_STAR    10
#define KEY_HASH    11
#define KEY_NONE    12
// Keyboards
#define KBD_SIDE_A  0
#define KBD_SIDE_B  1


// For event function: uint8_t AKbdSide, uint8_t AKey
typedef void(*ftVoidU8U8)(uint8_t, uint8_t);

class Keys_t {
private:
    uint32_t ITimer;
public:
    uint8_t PressedKey[2];
    void Task(void);
    void Init(void);
    ftVoidU8U8 EvtKbd;
};

extern Keys_t Keys;

#endif /* KEYS_H_ */
