/*
 * File:   media.h
 * Author: g.kruglov
 *
 * Created on June 15, 2011, 4:44 PM
 * Edited on June 05, 2018
 */

#include "kl_sd.h"
#include <stdint.h>
#include "kl_lib.h"
#include "shell.h"
#include "MsgQ.h"

// ==== Defines ====
// Command codes
#define VS_READ_OPCODE  0b00000011
#define VS_WRITE_OPCODE 0b00000010

// Registers
#define VS_REG_MODE         0x00
#define VS_REG_STATUS       0x01
#define VS_REG_BASS         0x02
#define VS_REG_CLOCKF       0x03
#define VS_REG_DECODE_TIME  0x04
#define VS_REG_AUDATA       0x05
#define VS_REG_WRAM         0x06
#define VS_REG_WRAMADDR     0x07
#define VS_REG_IN0          0x08
#define VS_REG_IN1          0x09
#define VS_REG_AIADDR       0x0A
#define VS_REG_VOL          0x0B
#define VS_REG_MIXERVOL     0x0C
#define VS_REG_RECCTRL      0x0D

#define VS_MAX_SPI_BAUDRATE_HZ  3400000

enum sndState_t {sndStopped, sndPlaying, sndWritingZeroes};

union VsMsg_t {
    uint8_t ID;
    VsMsg_t& operator = (const VsMsg_t &Right) {
        ID = Right.ID;
        return *this;
    }
    VsMsg_t() : ID(0) {}
    VsMsg_t(uint8_t AID) : ID(AID) {}
} __attribute__((__packed__));

#define VSMSG_STOP              1
#define VSMSG_DREQ_IRQ          2
#define VSMSG_DMA_DONE          3
#define VSMSG_COMPLETED         4
#define VSMSG_READ_NEXT         5

extern EvtMsgQ_t<VsMsg_t, MAIN_EVT_Q_LEN> EvtQVs;

#define VS_INITIAL_ATTENUATION  0x33
#define VS_DATA_BUF_SZ          1024    // bytes. Must be multiply of 512.
#define ZERO_SEQ_LEN            128     // After file end, send several zeroes


struct VsBuf_t {
    uint8_t Data[VS_DATA_BUF_SZ], *PData;
    UINT DataSz;
    FRESULT ReadFromFile(FIL *PFile) {
        PData = Data;   // Set pointer at beginning
        FRESULT rslt = f_read(PFile, Data, VS_DATA_BUF_SZ, &DataSz);
//        Uart.Printf("\rRead %u\r", DataSz);
        return rslt;
    }
};

// Event mask to wake from IRQ
#define VS_EVT_READ_NEXT    (eventmask_t)1

class Sound_t : public IrqHandler_t {
private:
    Spi_t ISpi{VS_SPI};
    VsBuf_t Buf1, Buf2, *PBuf;
    uint32_t ZeroesCount;
    FIL IFile;
    bool IDmaIdle;
    int16_t IAttenuation;
    const char* IFilename;
    uint32_t IStartPosition;
    // Pin operations
    inline void Rst_Lo()   { PinSetLo(VS_RST); }
    inline void Rst_Hi()   { PinSetHi(VS_RST); }
    inline void XCS_Lo()   { PinSetLo(VS_XCS); }
    inline void XCS_Hi()   { PinSetHi(VS_XCS);  }
    inline void XDCS_Lo()  { PinSetLo(VS_XDCS); }
    inline void XDCS_Hi()  { PinSetHi(VS_XDCS); }
    // Cmds
    uint8_t ReadCmd(uint8_t AAddr, uint16_t *AData);
    uint8_t WriteCmd(uint8_t AAddr, uint16_t AData);
    inline void StartTransmissionIfNotBusy() {
        chSysLock();
        if(IDmaIdle and IDreq.IsHi()) {
            PrintfI("TXinB\r");
            IDreq.EnableIrq(IRQ_PRIO_MEDIUM);
            IDreq.GenerateIrq();    // Do not call SendNexData directly because of its interrupt context
        }
        chSysUnlock();
    }
    void PrepareToStop();
    void SendZeroes();
    void IPlayNew();
public:
    sndState_t State;
    void Init();
    void Shutdown();
    void Play(const char* AFilename, uint32_t StartPosition = 0) {
        IFilename = AFilename;
        if(StartPosition & 1) StartPosition--;
        IStartPosition = StartPosition;
        EvtQVs.SendNowOrExit(VsMsg_t(VSMSG_STOP));
    }
    void Stop() {
        IFilename = NULL;
        EvtQVs.SendNowOrExit(VsMsg_t(VSMSG_STOP));
    }
    void AmplifierOn();
    void AmplifierOff();
    // 0...254
    void SetVolume(uint8_t AVolume) {
        if(AVolume == 0xFF) AVolume = 0xFE;
        IAttenuation = 0xFE - AVolume; // Transform Volume to attenuation
        WriteCmd(VS_REG_VOL, ((IAttenuation * 256) + IAttenuation));
    }

    uint32_t GetPosition() { return IFile.fptr; }
    // Inner use
    PinIrq_t IDreq{VS_DREQ, pudPullDown, this};
    thread_reference_t PThread;
    void IIrqHandler();
    void ITask();
    void ISendNextData();
};

extern Sound_t Sound;
