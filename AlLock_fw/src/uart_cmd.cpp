/*
 * uart_cmd.cpp
 *
 *  Created on: 20.02.2012
 *      Author: kreyl
 */

#include "uart_cmd.h"
#include "stm32f10x_rcc.h"
#include "delay_util.h"
#include <stdarg.h>

CmdUnit_t CmdUnit;

void CmdUnit_t::Init(void) {
    // ==== Clocks init ====
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);      // UART clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    // ==== GPIO init ====
    klGpio::SetupByN(GPIOA, 2, GPIO_Mode_AF_PP);       // TX2
    klGpio::SetupByN(GPIOA, 3, GPIO_Mode_IN_FLOATING); // RX2
    // ==== USART configuration ====
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);
    // ==== NVIC ====
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);     // Configure the NVIC Preemption Priority Bits
    // Enable the USART2 Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // Enable interrupts
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
    // Enable USART
    USART_Cmd(USART2, ENABLE);
    // ==== Coils ====
    klGpio::SetupByMsk(GPIOB, (GPIO_Pin_8 | GPIO_Pin_9), GPIO_Mode_AF_PP);
    CoilA.Init(TIM4, 7, TIM_FREQ_MAX, 3, TIM_OCPolarity_High);
    CoilA.Set(4);
    CoilB.Init(TIM4, 7, TIM_FREQ_MAX, 4, TIM_OCPolarity_High);
    CoilB.Set(4);
}

void CmdUnit_t::PrintUint(uint32_t ANumber) {
    uint8_t digit = '0';
    bool ShouldPrint = false;
    const uint32_t m[9] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10};

    for(uint8_t i=0; i<9; i++) {
        while (ANumber >= m[i]) {
            digit++;
            ANumber -= m[i];
        }
        if (digit != '0' || ShouldPrint) {
            BufWrite(digit);
            ShouldPrint = true;
        }
        digit = '0';
    } // for
    BufWrite((uint8_t)('0'+ANumber));
}

void CmdUnit_t::Print2Buf(const char *S, ...) {
    char c;
    bool WasPercent = false;
    va_list Arg;
    va_start(Arg, S);    // Set pointer to first argument
    while ((c = *S) != 0) {
        if (c == '%') {
            if (WasPercent) {
                BufWrite(c);  // %%
                WasPercent = false;
            }
            else WasPercent = true;
        }
        else { // not %
            if (WasPercent) {
                if (c == 'c') BufWrite((uint8_t)va_arg(Arg, int32_t));
                else if (c == 'u') PrintUint(va_arg(Arg, uint32_t));
                else if ((c == 's') || (c == 'S')) PrintString(va_arg(Arg, char*));
                WasPercent = false;
            } // if was percent
            else BufWrite(c);
        }
        S++;
    } // while
    va_end(Arg);
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

// ==== IRQ ====
void CmdUnit_t::IRQHandler() {
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        uint8_t b = USART_ReceiveData(USART2);
        switch (CmdState) {
            case csNone:
                RXBuf[RXCounter++] = b;
                CmdState = csInProgress;
                break;
            case csInProgress:
                // Check if end of cmd
                if (b == '\r') {
                    CmdState = csReady;
                    RXBuf[RXCounter] = 0;
                }
                else {
                    RXBuf[RXCounter++] = b;
                    // Check if too long
                    if (RXCounter == UART_BUF_SIZE) CmdReset();
                }
                break;
            case csReady:   // New byte received, but command still not handled
                break;
        } // switch
    } // if rx

    if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
        USART_SendData(USART2, BufRead());
        if (BufIsEmpty()) USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
    }
}


void USART2_IRQHandler(void) {
    CmdUnit.IRQHandler();
}

