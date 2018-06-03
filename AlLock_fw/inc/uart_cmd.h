/*
 * uart_cmd.h
 *
 *  Created on: 20.02.2012
 *      Author: kreyl
 */

#ifndef UART_CMD_H_
#define UART_CMD_H_

#include "stm32f10x_usart.h"
#include "kl_lib.h"


#define UART_BUF_SIZE    63

#define FIELD_GENERATOR_TIMEOUT 4005

enum CmdState_t {csNone, csInProgress, csReady};

class CmdUnit_t {
private:
    uint32_t ITimer;
    uint8_t TXBuf[UART_BUF_SIZE], WCounter, RCounter;
    klPwmChannel_t CoilA, CoilB;
    // Cmd
    CmdState_t CmdState;
    uint8_t RXBuf[UART_BUF_SIZE], RXCounter;
    void CmdReset(void) { RXCounter = 0; CmdState = csNone; }
    // Buffer
    void BufWrite(uint8_t AByte) {
        TXBuf[WCounter++] = AByte;
        if (WCounter == UART_BUF_SIZE) WCounter = 0;
    }
    uint8_t BufRead(void) {
        uint8_t IByte = TXBuf[RCounter++];
        if (RCounter == UART_BUF_SIZE) RCounter = 0;
        return IByte;
    }
    bool BufIsEmpty(void) { return (RCounter == WCounter); }
    // Printf
    void PrintUint(uint32_t ANumber);
    void PrintString (const char *S) { while (*S != '\0') BufWrite (*S++); }
public:
    void Print2Buf(const char *S, ...);
    void Init(void);
    void Task(void);
    // IRQ
    void IRQHandler(void);
};

// IRQ
extern "C" {
void USART2_IRQHandler(void);
}

extern CmdUnit_t CmdUnit;

#endif /* UART_CMD_H_ */
