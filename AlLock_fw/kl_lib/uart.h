/*
 * cmd_uart.h
 *
 *  Created on: 15.04.2013
 *      Author: kreyl
 */

#pragma once

#include "kl_lib.h"
#include <cstring>
#include "shell.h"
#include "board.h"

struct UartTxParams_t {
    uint32_t Baudrate;
    USART_TypeDef* Uart;
    GPIO_TypeDef *PGpioTx;
    uint16_t PinTx;
    // DMA
    const stm32_dma_stream_t *PDmaTx;
    uint32_t DmaModeTx;
    // MCU-specific
#if defined STM32F072xB || defined STM32L4XX
    bool UseIndependedClock;
#endif
    UartTxParams_t(uint32_t ABaudrate, USART_TypeDef* APUart, GPIO_TypeDef *APGpioTx, uint16_t APinTx,
    const stm32_dma_stream_t *APDmaTx, uint32_t ADmaModeTx
#if defined STM32F072xB || defined STM32L4XX
    , bool AUseIndependedClock;
#endif
    ) : Baudrate(ABaudrate), Uart(APUart), PGpioTx(APGpioTx), PinTx(APinTx), PDmaTx(APDmaTx), DmaModeTx(ADmaModeTx)
#if defined STM32F072xB || defined STM32L4XX
    , UseIndependedClock(AUseIndependedClock)
#endif
    {}
};

struct UartRxParams_t : public UartTxParams_t {
    GPIO_TypeDef *PGpioRx;
    uint16_t PinRx;
    const stm32_dma_stream_t *PDmaRx;
    uint32_t DmaModeRx;
};

#define UART_USE_DMA        TRUE
#define UART_USE_TXE_IRQ    FALSE

// Set to true if RX needed
#define UART_RX_ENABLED     TRUE

#if UART_RX_ENABLED // ==== RX ====
#define UART_RXBUF_SZ       99 // unprocessed bytes
#define UART_CMD_BUF_SZ     54 // payload bytes
#define UART_RX_POLLING_MS  99
#endif

// ==== Base class ====
class BaseUartTx_t {
private:
    const UartTxParams_t *Params;
#if UART_USE_DMA
    char TXBuf[UART_TXBUF_SZ];
    char *PRead, *PWrite;
    bool IDmaIsIdle;
    uint32_t IFullSlotsCount, ITransSize;
    void ISendViaDMA();
#endif
protected:
    uint8_t IPutByte(uint8_t b);
    uint8_t IPutByteNow(uint8_t b);
    void IStartTransmissionIfNotYet();
    virtual void IOnTxEnd() = 0;
    // ==== Constructor ====
    BaseUart_t(const UartTxParams_t *APParams) : Params(APParams)
#if UART_USE_DMA
    , PRead(TXBuf), PWrite(TXBuf), IDmaIsIdle(true), IFullSlotsCount(0), ITransSize(0)
#endif
    {}
public:
    void Init(uint32_t ABaudrate);
    void Shutdown();
    void OnClkChange();
    // Enable/Disable
    void EnableTx()  { Params->Uart->CR1 |= USART_CR1_TE; }
    void DisableTx() { Params->Uart->CR1 &= ~USART_CR1_TE; }
    void EnableRx()  { Params->Uart->CR1 |= USART_CR1_RE; }
    void DisableRx() { Params->Uart->CR1 &= ~USART_CR1_RE; }
#if UART_USE_DMA
    void FlushTx() { while(!IDmaIsIdle) chThdSleepMilliseconds(1); }  // wait DMA
#endif
    void EnableTCIrq(const uint32_t Priority, ftVoidVoid ACallback);
    // Inner use
#if UART_USE_DMA
    void IRQDmaTxHandler();
#endif
#if UART_RX_ENABLED
    uint32_t GetRcvdBytesCnt();
    uint8_t GetByte(uint8_t *b);
#endif
};

class BaseUartWithRx_t : public BaseUart_t {
private:
    int32_t OldWIndx, RIndx;
    uint8_t IRxBuf[UART_RXBUF_SZ];
public:
    BaseUartWithRx_t(const UartParams_t *APParams) :
        BaseUart_t(APParams), OldWIndx(0), RIndx(0) {}
};


class CmdUart_t : public BaseUart_t, public PrintfHelper_t, public Shell_t {
private:
    void IOnTxEnd() {} // Dummy
    uint8_t IPutChar(char c) { return IPutByte(c);  }
    void IStartTransmissionIfNotYet() { BaseUart_t::IStartTransmissionIfNotYet(); }
    void Printf(const char *format, ...) {
        va_list args;
        va_start(args, format);
        IVsPrintf(format, args);
        va_end(args);
    }
public:
    CmdUart_t(const UartParams_t *APParams) : BaseUart_t(APParams) {}
    void Init(uint32_t ABaudrate);
    void IRxTask();
    void SignalCmdProcessed();
};

#define BYTE_UART_EN    FALSE
#if BYTE_UART_EN
class ByteUart_t : public BaseUart_t, public ByteShell_t {
//private:
//    uint8_t IPutChar(char c) { return IPutByte(c);  }
//    void IStartTransmissionIfNotYet() { BaseUart_t::IStartTransmissionIfNotYet(); }
public:
    ByteUart_t(const UartParams_t *APParams) : BaseUart_t(APParams) {}
    void Init(uint32_t ABaudrate);
    uint8_t IPutChar(char c) { return IPutByte(c); }
    void IStartTransmissionIfNotYet() { BaseUart_t::IStartTransmissionIfNotYet(); }
    void IRxTask();
};
#endif
