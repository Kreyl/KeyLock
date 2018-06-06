#pragma once

#include <inttypes.h>
#include "kl_lib.h"
#include "MsgQ.h"

// Timings
#define BTNS_PERIOD_MS   45
// Keys
#define BTN_STAR    10
#define BTN_HASH    11
#define BTN_NONE    12
// Keyboards
#define KBD_SIDE_A  0
#define KBD_SIDE_B  1

void BtnsInit();
