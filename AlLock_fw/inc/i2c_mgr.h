/*
 * File:   i2c_mgr.h
 * Author: Kreyl
 *
 * Created on May 31, 2011, 8:34 PM
 */

#ifndef I2C_MGR_H
#define	I2C_MGR_H

#include "stm32f10x.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"
#include "misc.h"

#include "kl_util.h"

// Inner use defines
#define I2C_DMA_CHNL_TX         DMA1_Channel6
#define I2C_DMA_CHNL_RX         DMA1_Channel7
#define I2C_DMA_FLAG_TC_TX      DMA1_FLAG_TC6
#define I2C_DMA_FLAG_TC_RX      DMA1_FLAG_TC7

// ================================= Constants =================================
#define I2C_CMD_QUEUE_LENGTH    9
#define I2C_TIMEOUT             50004
#define I2C_TIMEOUT_MS          4

// Error codes
#define I2C_OK                  0
#define I2C_ERR_TIMEOUT         1
#define I2C_ERR_NOMASTER        2
#define I2C_ERR_SLAVE_NACK      3
#define I2C_WAITING             4


// ================================ Data types =================================
// Struct to write or read single register
struct SingleReg_t {
    uint8_t RegAddr;
    uint8_t RegValue;
};

struct Buf8_t {
    uint8_t *Buf;
    uint8_t Length;
    uint8_t CurrentItem;
} __attribute__ ((packed));

enum i2cCmdState_t {CmdPending, CmdWritingAddrTX, CmdWritingAddrRX, CmdWritingOne, CmdWritingMany, CmdReadingOne, CmdReadingMany, CmdSucceded, CmdFailed};

struct I2C_Cmd_t {
    Buf8_t DataToWrite, DataToRead;     // Buffers of data to read or write
    uint8_t Address;                    // Device address
    i2cCmdState_t State;
    ftVoid_Void Callback;
};

class i2cMgr_t {
private:
    uint32_t Timer;
    bool IsError;
    I2C_Cmd_t *CmdToWrite, *CmdToRead;
    I2C_Cmd_t Commands[I2C_CMD_QUEUE_LENGTH];
    // Task-based functions
    void SendAddrTX(void);
    uint8_t CheckAddrTXSending(void);
    void SendAddrRX(void);
    uint8_t CheckAddrRXSending(void);
    void WriteOneByte(void);
    uint8_t CheckOneByteWriting(void);
    void WriteMany(void);
    uint8_t CheckManyWriting(void);
    void ReadOneByte(void);
    uint8_t CheckOneByteReading(void);
    void ReadMany(void);
    uint8_t CheckManyReading(void);
    void GetNext(void);
    void StopAndGetNext(void);
    // Polling-based functions
    uint8_t BusyWait(void);
    uint8_t SendStart(void);
    uint8_t SendAddrTXPoll(uint8_t AAddr);
    uint8_t SendAddrRXPoll(uint8_t AAddr);
    uint8_t WriteOneBytePoll(void);
public:
    void Init(void);
    void Task(void);
    void WriteBufferPoll(uint8_t AAddr, uint8_t *ABuffer, uint8_t ABufferSize);
    void AddCmd(I2C_Cmd_t ACmd);
};

extern i2cMgr_t i2cMgr;


#endif	/* I2C_MGR_H */

