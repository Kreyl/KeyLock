/*
 * keys.cpp
 *
 *  Created on: 08.02.2012
 *      Author: kreyl
 */

#include "btns.h"
#include "kl_lib.h"

const uint16_t BtnOuts[2][3] = { {BTN_A1_PIN, BTN_A2_PIN, BTN_A3_PIN}, {BTN_B1_PIN, BTN_B2_PIN, BTN_B3_PIN} };

static uint8_t PressedBtn[2];

static THD_WORKING_AREA(waBtnThread, 128);
__noreturn
static void BtnThread(void *arg) {
    chRegSetThreadName("Btn");
    while(true) {
        chThdSleepMilliseconds(BTNS_PERIOD_MS);
        uint8_t BtnPressedNow;
        // Iterate keyboards
        for(uint8_t k=0; k<2; k++) {
            BtnPressedNow = BTN_NONE;
            // Iterate outputs
            for (uint8_t o=0; o<3; o++) {
                // Make needed output to be output
                PinSetupModeOut(BTNS_GPIO, BtnOuts[k][o], omOpenDrain);
                PinSetLo(BTNS_GPIO, BtnOuts[k][o]);
                // Read current input state
                if      (PinIsLo(BTNS_GPIO, BTN_4_PIN)) BtnPressedNow = 1+o;
                else if (PinIsLo(BTNS_GPIO, BTN_5_PIN)) BtnPressedNow = 4+o;
                else if (PinIsLo(BTNS_GPIO, BTN_6_PIN)) BtnPressedNow = 7+o;
                else if (PinIsLo(BTNS_GPIO, BTN_7_PIN)) BtnPressedNow = (o == 0)? BTN_STAR : ((o == 1)? 0 : BTN_HASH);
                // Make all outputs to be analog in
                PinSetupModeAnalog(BTNS_GPIO, BtnOuts[k][o]);
            } // for o
            // Trigger event if key changed
            if (BtnPressedNow != PressedBtn[k]) {
                PressedBtn[k] = BtnPressedNow;
                if(BtnPressedNow != BTN_NONE)
                    EvtQMain.SendNowOrExit(EvtMsg_t(evtIdButtons, k, BtnPressedNow));
            }
        } // for k
    }
}

void BtnsInit() {
    // Inputs
    PinSetupInput(BTNS_GPIO, BTN_4_PIN, pudPullUp);
    PinSetupInput(BTNS_GPIO, BTN_5_PIN, pudPullUp);
    PinSetupInput(BTNS_GPIO, BTN_6_PIN, pudPullUp);
    PinSetupInput(BTNS_GPIO, BTN_7_PIN, pudPullUp);
    // Outputs (will be set up inside a task)
    PinSetupAnalog(BTNS_GPIO, BTN_A1_PIN);
    PinSetupAnalog(BTNS_GPIO, BTN_A2_PIN);
    PinSetupAnalog(BTNS_GPIO, BTN_A3_PIN);
    PinSetupAnalog(BTNS_GPIO, BTN_B1_PIN);
    PinSetupAnalog(BTNS_GPIO, BTN_B2_PIN);
    PinSetupAnalog(BTNS_GPIO, BTN_B3_PIN);
    // Reset pressed key
    PressedBtn[0] = BTN_NONE;
    PressedBtn[1] = BTN_NONE;
    // Thread
    chThdCreateStatic(waBtnThread, sizeof(waBtnThread), NORMALPRIO, (tfunc_t)BtnThread, NULL);
}
