/*
 * File:   vs.h
 * Author: Kreyl
 *
 * Created on June 9, 2011, 11:46 AM
 */

#ifndef VS_H
#define	VS_H

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_dma.h"
#include "misc.h"

#include "vs_defs.h"

// ==== Defines ====
// Pins
#define VS_XCS          GPIO_Pin_0
#define VS_XDCS         GPIO_Pin_4
#define VS_RST          GPIO_Pin_15
#define VS_DREQ         GPIO_Pin_1
#define VS_XCLK         GPIO_Pin_5
#define VS_SO           GPIO_Pin_6
#define VS_SI           GPIO_Pin_7

// Constants
#define VS_TIMEOUT              8000000
//#define VS_BUFSIZE              512
#define VS_TRAILING_0_COUNT     11

// Types
typedef struct {
    uint8_t Instruction;
    uint8_t Address;
    uint16_t Data;
} VSCmd_t;

class VS_t {
private:
    uint8_t IZero;
    uint8_t ReadWriteByte(uint8_t AByte);

    void Rst_Lo(void)  { GPIOA->BRR  = VS_RST;  }
    void Rst_Hi(void)  { GPIOA->BSRR = VS_RST;  }
    void XCS_Lo(void)  { GPIOA->BRR  = VS_XCS;  }
    void XCS_Hi(void)  { GPIOA->BSRR = VS_XCS;  }

    uint8_t BusyWait(void);
    uint8_t CmdRead(uint8_t AAddr, uint16_t *AData);
    uint8_t CmdWrite(uint8_t AAddr, uint16_t AData);
public:
    bool DMA_Busy;
    // Low-level
    void XDCS_Lo(void) { GPIOA->BRR  = VS_XDCS; }
    void XDCS_Hi(void) { GPIOA->BSRR = VS_XDCS; }
    bool IsBusy (void);
    // General
    void Init(void);
    void Enable(void);
    void Disable(void);
    void AmplifierOn(void);
    void AmplifierOff(void);
    // Playback
    void WriteData(uint8_t *ABuf, uint16_t ACount);
    void WriteTrailingZeroes(void);
    void SetVolume(uint8_t Attenuation) { CmdWrite(VS_REG_VOL, ((Attenuation*256)+Attenuation)); }
    void Stop(void) { CmdWrite(VS_REG_RECCTRL, VS_SARC_OUTOFADPCM); }
};

extern VS_t Vs;

#endif	/* VS_H */

