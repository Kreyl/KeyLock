/*
 * main.h
 *
 *  Created on: 28.01.2012
 *      Author: kreyl
 */

#ifndef MAIN_H_
#define MAIN_H_


enum CmdState_t {csNone, csInProgress, csReady};
#define CMD_SIZE    63
class CmdRx_t {
private:
    CmdState_t CmdState;
    uint8_t IBuf[CMD_SIZE], ICounter;
    void CmdReset(void) { ICounter = 0; CmdState = csNone; }
public:
    void NewByte(uint8_t AByte);
    void Task(void);
};


#define BUF_SIZE    144
class UartBuf_t {
private:
    uint8_t IBuf[BUF_SIZE], CmdBuf[63], CmdRxCounter;
    uint8_t WCounter, RCounter;
public:
    void Init(void) { WCounter = 0; RCounter = 0; }
    void Add(uint8_t AByte);
    void Add(uint8_t *AArr, uint8_t ALength) { for(uint8_t i=0; i<ALength; i++) Add(*AArr++); }
    void Task(void);
};

// Prototypes
void GeneralInit(void);
void KLUartInit(void);

// IRQs
extern "C" {
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
}

#endif /* MAIN_H_ */
