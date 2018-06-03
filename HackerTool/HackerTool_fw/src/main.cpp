/*
 * File:   main.cpp
 * Author: Kreyl
 *
 * Created on May 27, 2011, 6:37 PM
 */

#include "stm32f10x.h"
#include "delay_util.h"
#include "kl_util.h"
#include "kl_lib.h"
#include "main.h"
#include "led.h"
#include "adc.h"

UartBuf_t Buf;
CmdRx_t CmdRx;
// Side detection pin
klPin_t PinSide;

int main(void) {
    GeneralInit();

    while (1) {
        Led1.Task();
        Led2.Task();
        Buf.Task();
        CmdRx.Task();
    } // while 1
}

void GeneralInit(void) {
    Delay.Init();
    Delay.ms(63);
    UART_Init();

    KLUartInit();
    Adc.Init();
    Buf.Init();
    PinSide.Init(GPIOA, 4, GPIO_Mode_IPU);

    Led1.Init(GPIOB, GPIO_Pin_10);
    Led2.Init(GPIOB, GPIO_Pin_11);
    Led1.Blink();
    Led2.Blink();

    klPrintf("\rHacker Tool v.0.21\r");
}


// ================================ IRQs =======================================
void USART1_IRQHandler(void) {
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        // Read one byte from the receive data register
        uint16_t b = USART_ReceiveData(USART1);
        CmdRx.NewByte(b);
    } // if rx
}

void USART2_IRQHandler(void) {
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        // Read one byte from the receive data register
        uint8_t b = USART_ReceiveData(USART2);
        USART_SendData(USART1, b);  // Send immediately
    } // if rx
}

// =============================== USARTs ======================================
void KLUartInit(void) {
    // ==== Clocks ====
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);      // UART clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);      // UART clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    // ==== GPIO ====
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    // TX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_2;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // ==== USART configuration ====
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    // USART1
    USART_InitStructure.USART_BaudRate = 128000;
    USART_Init(USART1, &USART_InitStructure);
    // USART2
    USART_InitStructure.USART_BaudRate = 9600;
    USART_Init(USART2, &USART_InitStructure);

    // ==== NVIC ====
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);     // Configure the NVIC Preemption Priority Bits
    // Enable the USART1 Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // Enable the USART2 Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    // Enable USARTs
    USART_Cmd(USART1, ENABLE);
    USART_Cmd(USART2, ENABLE);
}

// ================================= CmdRx_t ===================================
void CmdRx_t::NewByte(uint8_t AByte) {
    switch (CmdState) {
        case csNone:
            IBuf[ICounter++] = AByte;
            CmdState = csInProgress;
            break;
        case csInProgress:
            // Check if end of cmd
            if (AByte == '\r') {
                CmdState = csReady;
                IBuf[ICounter] = 0;
            }
            else {
                IBuf[ICounter++] = AByte;
                // Check if too long
                if (ICounter == CMD_SIZE) CmdReset();
            }
            break;
        case csReady:   // New byte received, but command still not handled
            break;
    } // switch
}

void CmdRx_t::Task() {
    if (CmdState == csReady) {
        // ==== Tool commands ====
        if (IBuf[0] == 'P') {
            klPrintf("HackerTool v0.21\r");
            Led1.Blink();
        }
        else if (IBuf[0] == 'U') {
            klPrintf("U:%u\r", Adc.Measure());
            Led1.Blink();
        }
        else if (IBuf[0] == 'N') {    // NOP: indicate some transmission
            klPrintf("N\r");
            Led2.Blink();
        }
        else if (IBuf[0] == 'G') {    // Get connected side
            if (PinSide == 1) klPrintf("G:A\r");    // Side A connected
            else klPrintf("G:B\r");                 // Side B connected
        }

        // ==== Lock emulator ====
        /*
        else if (b == 'S') {    // Get state
            klPrintf("S:123456,1234,,3\r");
            Led2.Blink();
        }
        else if (b == 'O') {    // Open door
            klPrintf("O\r");
            Led2.Blink();
        }
        else if (b == 'C') {    // Close door
            klPrintf("C\r");
            Led2.Blink();
        }
        else if (b == 'A') {    // Replace codeA, not implemented
            klPrintf("A\r");
            Led2.Blink();
        }
        else if (b == 'B') {    // Replace codeB, not implemented
            klPrintf("B\r");
            Led2.Blink();
        }
        else if (b == 'V') {    // Replace service code, not implemented
            klPrintf("V\r");
            Led2.Blink();
        }
        // */

        // Redirect command to lock
        else {
            Led2.Blink();
            Buf.Add(IBuf, ICounter);
            Buf.Add('\r');
        }
        CmdReset();
    } // is ready
}

// ================================= UartBuf_t =================================
void UartBuf_t::Add(uint8_t AByte) {
    IBuf[WCounter++] = AByte;
    if (WCounter == BUF_SIZE) WCounter = 0;
}
void UartBuf_t::Task(void) {
    // Check if may write
    if (USART_GetFlagStatus(USART2, USART_FLAG_TC) == SET) {    // Transmission complete
        if (RCounter != WCounter) {
            USART_SendData(USART2, IBuf[RCounter++]);
            if (RCounter == BUF_SIZE) RCounter = 0;
        } // if not empty
    } // if transmission complete
}

