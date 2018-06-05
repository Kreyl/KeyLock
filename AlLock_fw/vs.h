/*
 * File:   vs.h
 * Author: Kreyl
 *
 * Created on June 9, 2011, 11:46 AM
 */

#pragma once

#include "kl_lib.h"
#include "board.h"
#include "vs_defs.h"

// Constants
#define VS_MAX_SPI_BAUDRATE_HZ  3400000
#define VS_TIMEOUT              8000000
//#define VS_BUFSIZE              512
#define VS_TRAILING_0_COUNT     11

// Types
typedef struct {
    uint8_t Instruction;
    uint8_t Address;
    uint16_t Data;
} VSCmd_t;

class VS1011_t : public IrqHandler_t  {
private:
    Spi_t ISpi{VS_SPI};
    uint8_t IZero;
    PinIrq_t IDreq{VS_DREQ, pudPullDown, this};
    void Rst_Lo()  { PinSetLo(VS_RST); }
    void Rst_Hi()  { PinSetHi(VS_RST); }
    void XCS_Lo()  { PinSetLo(VS_XCS); }
    void XCS_Hi()  { PinSetHi(VS_XCS); }

    uint8_t BusyWait();
    uint8_t CmdRead(uint8_t AAddr, uint16_t *AData);
    uint8_t CmdWrite(uint8_t AAddr, uint16_t AData);
public:
    bool IDmaIsIdle;
    // Low-level
    void XDCS_Lo() { PinSetLo(VS_XDCS); }
    void XDCS_Hi() { PinSetHi(VS_XDCS); }
    bool IsBusy();
    // General
    void Init();
    void AmplifierOn();
    void AmplifierOff();
    void ISendNexChunk();
    void IIrqHandler();

    // Playback
    void WriteData(uint8_t *ABuf, uint16_t ACount);
    void WriteTrailingZeroes();
    void SetVolume(uint8_t Attenuation) { CmdWrite(VS_REG_VOL, ((Attenuation*256)+Attenuation)); }
    void Stop() { CmdWrite(VS_REG_RECCTRL, VS_SARC_OUTOFADPCM); }
};

extern VS1011_t Vs;
