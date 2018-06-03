/*
 * keys.cpp
 *
 *  Created on: 08.02.2012
 *      Author: kreyl
 */

#include "keys.h"
#include "kl_lib.h"

Keys_t Keys;
const uint16_t KeybOuts[2][3] = { {GPIO_Pin_1, GPIO_Pin_2, GPIO_Pin_10}, {GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_5} };


void Keys_t::Task() {
    if (!Delay.Elapsed(&ITimer, KEYS_RATE)) return;
    uint8_t KeyPressedNow;

    // Iterate keyboards
    for (uint8_t k=0; k<2; k++) {
        KeyPressedNow = KEY_NONE;
        // Iterate outputs
        for (uint8_t o=0; o<3; o++) {
            // Make all outputs to be inputs
            klGpio::SetupByMsk(GPIOB, (GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_10), GPIO_Mode_AIN);
            // Make needed output to be output
            klGpio::SetupByMsk(GPIOB, KeybOuts[k][o], GPIO_Mode_Out_OD);
            // Read current input state
            if      (klGpio::IsClearByN(GPIOB, 11)) KeyPressedNow = 1+o;
            else if (klGpio::IsClearByN(GPIOB, 12)) KeyPressedNow = 4+o;
            else if (klGpio::IsClearByN(GPIOB, 13)) KeyPressedNow = 7+o;
            else if (klGpio::IsClearByN(GPIOB, 14)) KeyPressedNow = (o == 0)? KEY_STAR : ((o == 1)? 0 : KEY_HASH);
        } // for o
        // Trigger event if key changed
        if (KeyPressedNow != PressedKey[k]) {
            PressedKey[k] = KeyPressedNow;
            if ((KeyPressedNow != KEY_NONE)  && (EvtKbd != 0)) EvtKbd(k, KeyPressedNow);
        }
    } // for k
}

void Keys_t::Init() {
    // Inputs
    klGpio::SetupByMsk(GPIOB, (GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14), GPIO_Mode_IPU);
    // Outputs (will be set up inside a task)
    klGpio::ClearByMsk(GPIOB, (GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_10));
    // Reset pressed key
    PressedKey[0] = KEY_NONE;
    PressedKey[1] = KEY_NONE;
}
