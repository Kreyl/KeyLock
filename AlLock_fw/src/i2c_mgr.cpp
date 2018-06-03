#include "i2c_mgr.h"
#include "delay_util.h"

i2cMgr_t i2cMgr;

// ============================ Tasking and handling ===========================
void i2cMgr_t::Task() {
    if (IsError) {
        Init();
        return;
    }
    uint8_t IEvt;
    switch (CmdToRead->State) {
        case CmdPending:
            if (SendStart() == I2C_OK) {    // Send Start
                // Send Addr of needed type
                if (CmdToRead->DataToWrite.Length != 0) {
                    SendAddrTX();
                    CmdToRead->State = CmdWritingAddrTX;
                }
                else {  // nothing to write
                    CmdToRead->State = CmdSucceded;
                    StopAndGetNext();
                }
            } // if start
            else {
                CmdToRead->State = CmdFailed;
                StopAndGetNext();
            }
            break;

        case CmdWritingAddrTX:  // ****** Writing Address with TX bit *******
            IEvt = CheckAddrTXSending();        // Check if ok
            if (IEvt == I2C_OK) {               // Addr sent, write data
                if (CmdToRead->DataToWrite.Length == 1) {   // Single byte to write
                    WriteOneByte();
                    CmdToRead->State = CmdWritingOne;
                }
                else {  // Many bytes to write
                    WriteMany();
                    CmdToRead->State = CmdWritingMany;
                }
            }
            else if (IEvt != I2C_WAITING) { // Some error occured
                if (IEvt == I2C_ERR_TIMEOUT) klPrintf("I2C Addr TX timeout\r");
                else if (IEvt == I2C_ERR_SLAVE_NACK) klPrintf("I2C NACK\r");
                CmdToRead->State = CmdFailed;
                StopAndGetNext();
            }
            break;  // Otherwise still sending address

        case CmdWritingOne: // ****** Writing one byte *******
            IEvt = CheckOneByteWriting();
            if (IEvt == I2C_OK) {               // Byte sent, check if reading needed
                if (CmdToRead->DataToRead.Length != 0) {    // Send RepeatedStart
                    if (SendStart() == I2C_OK) {
                        SendAddrRX();
                        CmdToRead->State = CmdWritingAddrRX;
                    }
                    else {  // Repeated start failed
                        CmdToRead->State = CmdFailed;
                        StopAndGetNext();
                        klPrintf("I2C OneByte RS failed\r");
                    }
                }
                else {  // nothing to read
                    CmdToRead->State = CmdSucceded;
                    if(CmdToRead->Callback != 0) CmdToRead->Callback();
                    StopAndGetNext();
                    klPrintf("I2C OneByte no read\r");
                }
            } //
            else if (IEvt != I2C_WAITING) { // Some error occured
                CmdToRead->State = CmdFailed;
                StopAndGetNext();
                klPrintf("I2C OneByte timeout\r");
            }
            break;  // Otherwise still sending byte

        case CmdWritingAddrRX:  // ****** Writing Address with RX bit *******
            IEvt = CheckAddrRXSending();        // Check if ok
            if (IEvt == I2C_OK) {               // Addr sent, read data
                if (CmdToRead->DataToRead.Length == 1) {   // Single byte to read
                    ReadOneByte();
                    CmdToRead->State = CmdReadingOne;
                }
                else {  // Many bytes to read
                    ReadMany();
                    CmdToRead->State = CmdReadingMany;
                }
            }
            else if (IEvt != I2C_WAITING) { // Some error occured
                CmdToRead->State = CmdFailed;
                StopAndGetNext();
                klPrintf("I2C Addr RX timeout\r");
            }
            break;  // Otherwise still sending address

        case CmdReadingOne: // ****** Reading one byte *******
            IEvt = CheckOneByteReading();
            if (IEvt == I2C_OK) {               // Byte read
                CmdToRead->State = CmdSucceded;
                if(CmdToRead->Callback != 0) CmdToRead->Callback();
                GetNext();  // No need to send stop as it sent automatically
            }
            else if (IEvt != I2C_WAITING) { // Some error occured
                CmdToRead->State = CmdFailed;
                StopAndGetNext();
            }
            break;  // Otherwise still reading byte

        case CmdWritingMany: // ****** Writing many bytes *******
            IEvt = CheckManyWriting();
            if (IEvt == I2C_OK) {               // Bytes sent, check if reading needed
                if (CmdToRead->DataToRead.Length != 0) {    // Send RepeatedStart and address RX
                    if (SendStart() == I2C_OK) {            // Send Start
                        SendAddrRX();
                        CmdToRead->State = CmdWritingAddrRX;
                    }
                    else {  // Repeated start failed
                        CmdToRead->State = CmdFailed;
                        StopAndGetNext();
                    }
                }
                else {  // nothing to read
                    CmdToRead->State = CmdSucceded;
                    if(CmdToRead->Callback != 0) CmdToRead->Callback();
                    StopAndGetNext();
                }
            } //
            else if (IEvt != I2C_WAITING) { // Some error occured
                CmdToRead->State = CmdFailed;
                StopAndGetNext();
            }
            break;  // Otherwise still sending bytes

        case CmdReadingMany: // ****** Reading many bytes *******
            IEvt = CheckManyReading();
            if (IEvt == I2C_OK) {               // Byte read
                CmdToRead->State = CmdSucceded;
                if(CmdToRead->Callback != 0) CmdToRead->Callback();
                StopAndGetNext();
            }
            else if (IEvt != I2C_WAITING) { // Some error occured
                CmdToRead->State = CmdFailed;
                StopAndGetNext();
                klPrintf("I2C ReadMany timeout\r");
            }
            break;  // Otherwise still reading bytes

        case CmdFailed:
        case CmdSucceded:
            GetNext();
            break;
    } // switch
}

void i2cMgr_t::SendAddrTX() {
    I2C_Send7bitAddress(I2C1, ((CmdToRead->Address) << 1), I2C_Direction_Transmitter);
    Delay.Reset(&Timer);
}
uint8_t i2cMgr_t::CheckAddrTXSending() {
    uint32_t IEvt = I2C_GetLastEvent(I2C1);
    if (IEvt == I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) return I2C_OK;
    if (IEvt & I2C_EVENT_SLAVE_ACK_FAILURE)                 return I2C_ERR_SLAVE_NACK;   // NACK occured, slave doesn't answer
    if (Delay.Elapsed(&Timer, I2C_TIMEOUT_MS))              return I2C_ERR_TIMEOUT;
    return I2C_WAITING;
}

void i2cMgr_t::SendAddrRX() {
    I2C_Send7bitAddress(I2C1, ((CmdToRead->Address) << 1), I2C_Direction_Receiver);
    Delay.Reset(&Timer);
}
uint8_t i2cMgr_t::CheckAddrRXSending() {
    uint32_t IEvt = I2C_GetLastEvent(I2C1);
    if (IEvt == I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) return I2C_OK;
    if (IEvt & I2C_EVENT_SLAVE_ACK_FAILURE)              return I2C_ERR_SLAVE_NACK;   // NACK occured, slave doesn't answer
    if (Delay.Elapsed(&Timer, I2C_TIMEOUT_MS))           return I2C_ERR_TIMEOUT;
    return I2C_WAITING;
}

void i2cMgr_t::WriteOneByte() {
    I2C_SendData(I2C1, CmdToRead->DataToWrite.Buf[0]);
    Delay.Reset(&Timer);
}
uint8_t i2cMgr_t::CheckOneByteWriting() {
    uint32_t IEvt = I2C_GetLastEvent(I2C1);
    if (IEvt == I2C_EVENT_MASTER_BYTE_TRANSMITTED) return I2C_OK;
    if (IEvt & I2C_EVENT_SLAVE_ACK_FAILURE)        return I2C_ERR_SLAVE_NACK;   // NACK occured, slave doesn't answer
    if (Delay.Elapsed(&Timer, I2C_TIMEOUT_MS))     return I2C_ERR_TIMEOUT;
    return I2C_WAITING;
}

void i2cMgr_t::ReadOneByte() {
    // Disable Acknowledgement, as only one byte is to be received
    I2C_AcknowledgeConfig(I2C1, DISABLE);
    // Send STOP Condition and then read byte
    I2C_GenerateSTOP(I2C1, ENABLE);
    Delay.Reset(&Timer);
}
uint8_t i2cMgr_t::CheckOneByteReading() {
    uint32_t IEvt = I2C_GetLastEvent(I2C1);
    if (IEvt == I2C_EVENT_MASTER_BYTE_RECEIVED) {
        CmdToRead->DataToRead.Buf[0] = I2C_ReceiveData(I2C1);
        return I2C_OK;
    }
    if (Delay.Elapsed(&Timer, I2C_TIMEOUT_MS)) return I2C_ERR_TIMEOUT;
    return I2C_WAITING;
}

void i2cMgr_t::WriteMany() {
    // Prepare DMA
    DMA_DeInit(I2C_DMA_CHNL_TX);
    DMA_InitTypeDef DMA_InitStructure;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &I2C1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr     = (uint32_t) &CmdToRead->DataToWrite.Buf[0];
    DMA_InitStructure.DMA_BufferSize         = CmdToRead->DataToWrite.Length;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_High;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralDST;  // From memory to I2C
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(I2C_DMA_CHNL_TX, &DMA_InitStructure);
    // Start transmission
    DMA_Cmd(I2C_DMA_CHNL_TX, ENABLE);   // Enable DMA channel
    I2C_DMACmd(I2C1, ENABLE);           // Enable DMA
    //Uart.PrintString("ST\r\r");
    Delay.Reset(&Timer);
}
uint8_t i2cMgr_t::CheckManyWriting() {
    // Check if DMA transfer ended
    if (DMA_GetFlagStatus(I2C_DMA_FLAG_TC_TX) == SET)
        if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) == RESET)
            return I2C_OK;

    if (Delay.Elapsed(&Timer, I2C_TIMEOUT_MS)) return I2C_ERR_TIMEOUT;
    return I2C_WAITING;
}

void i2cMgr_t::ReadMany() {
    // Enable Acknowledgement
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    // Prepare DMA
    DMA_DeInit(I2C_DMA_CHNL_RX);
    DMA_InitTypeDef DMA_InitStructure;
    //DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &I2C1->DR;
    DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40005410;
    DMA_InitStructure.DMA_MemoryBaseAddr     = (uint32_t) &CmdToRead->DataToRead.Buf[0];
    DMA_InitStructure.DMA_BufferSize         = CmdToRead->DataToRead.Length;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;  // From I2C to memory
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(I2C_DMA_CHNL_RX, &DMA_InitStructure);
    //Inform the DMA that the next End Of Transfer Signal will be the last one, need to send NACK after last byte
    I2C_DMALastTransferCmd(I2C1, ENABLE);
    // Start transmission
    I2C_DMACmd(I2C1, ENABLE);           // Enable DMA
    DMA_Cmd(I2C_DMA_CHNL_RX, ENABLE);   // Enable DMA channel

    Delay.Reset(&Timer);
}
uint8_t i2cMgr_t::CheckManyReading() {
    // Check if DMA transfer ended
    if (DMA_GetFlagStatus(I2C_DMA_FLAG_TC_RX) == SET) return I2C_OK;
    if (Delay.Elapsed(&Timer, I2C_TIMEOUT_MS))        return I2C_ERR_TIMEOUT;
    return I2C_WAITING;
}

void i2cMgr_t::GetNext() {
    CmdToRead++;
    if ((CmdToRead - &Commands[0]) >= I2C_CMD_QUEUE_LENGTH) CmdToRead = &Commands[0];
}
void i2cMgr_t::StopAndGetNext() {
    I2C_GenerateSTOP(I2C1, ENABLE);
    GetNext();
}

// ================================ Init & Add Cmd =============================
void i2cMgr_t::Init() {
    // ==== Init I2C ====
    // Init clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);    // I2C clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);   // Peripheral clock
    // Init GPIO - SCL & SDA
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    // I2C
    I2C_DeInit(I2C1);
    I2C_InitTypeDef I2C_InitStructure;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x30;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;
    I2C_Init(I2C1, &I2C_InitStructure);
    I2C_Cmd(I2C1, ENABLE);

    // ==== Init manager ====
    CmdToWrite = &Commands[0];
    CmdToRead  = &Commands[0];
    // Reset stage of all commands
    for (uint8_t i=0; i<I2C_CMD_QUEUE_LENGTH; i++) {
        Commands[i].State = CmdSucceded;
        Commands[i].Callback = 0;
    }
    // ==== Init DMA ====
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    IsError = false;
}

void i2cMgr_t::AddCmd(I2C_Cmd_t ACmd) {
    ACmd.State = CmdPending;
    *CmdToWrite = ACmd;     // Copy command to buffer
    // DEBUG
//    UART_PrintString("Cmd: ");
//    UART_PrintArrAsHex(ACmd.DataToWrite.Buf, ACmd.DataToWrite.Length);
    // Process command queue
    CmdToWrite++;
    if ((CmdToWrite - &Commands[0]) >= I2C_CMD_QUEUE_LENGTH) CmdToWrite = &Commands[0];
}

// =========================== Polling functions ===============================
// Wait for Idle Bus
uint8_t i2cMgr_t::BusyWait() {
    uint32_t FTimeout = I2C_TIMEOUT;
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
        if((FTimeout--) == 0) {
            IsError = true;
            klPrintf("I2C BusyWait timeout\r");
            return I2C_ERR_TIMEOUT;
        }
    return I2C_OK;
}
uint8_t i2cMgr_t::SendStart(void) {
    I2C_GenerateSTART(I2C1, ENABLE);
    uint32_t FTimeout = I2C_TIMEOUT;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) {
        if ((FTimeout--) == 0) {
            klPrintf("I2C can't enter master mode\r");
            IsError = true;
            return I2C_ERR_NOMASTER;
        }
    }
    return I2C_OK;
}
uint8_t i2cMgr_t::SendAddrTXPoll(uint8_t AAddr) {
    uint32_t IEvt;
    I2C_Send7bitAddress(I2C1, (AAddr << 1), I2C_Direction_Transmitter);
    uint32_t FTimeout = I2C_TIMEOUT;
    do {
        IEvt = I2C_GetLastEvent(I2C1);
        if (IEvt & I2C_EVENT_SLAVE_ACK_FAILURE) {   // NACK occured, slave doesn't answer
            klPrintf("I2C Slave Addr NACK\r");
            return I2C_ERR_SLAVE_NACK;
        }
        if ((FTimeout--) == 0) {
            klPrintf("I2C Slave Addr Timeout\r");
            IsError = true;
            return I2C_ERR_TIMEOUT;
        }
    } while (IEvt != I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
    return I2C_OK;   // all right
}
uint8_t i2cMgr_t::SendAddrRXPoll(uint8_t AAddr) {
    uint32_t IEvt;
    I2C_Send7bitAddress(I2C1, AAddr, I2C_Direction_Receiver);
    uint32_t FTimeout = I2C_TIMEOUT;
    do {
        IEvt = I2C_GetLastEvent(I2C1);
        if (IEvt & I2C_EVENT_SLAVE_ACK_FAILURE) {   // NACK occured, slave doesn't answer
            klPrintf("I2C Slave Addr NACK\r");
            return I2C_ERR_SLAVE_NACK;
        }
        if ((FTimeout--) == 0) {
            klPrintf("I2C Slave Addr Timeout\r");
            IsError = true;
            return I2C_ERR_TIMEOUT;
        }
    } while (IEvt != I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);
    return I2C_OK;   // all right
}

void i2cMgr_t::WriteBufferPoll(uint8_t AAddr, uint8_t* ABuffer, uint8_t ABufferSize) {
    //klPrintf("I2C write buf addr = %X\r", AAddr);
    uint32_t IEvt;
    uint8_t b;
    if (BusyWait()) return;
    // Send Start and Address as transmitter
    if (SendStart()) return;
    if (SendAddrTXPoll(AAddr)) return;
    // Address was acknowledged, start transmission
    for (uint8_t i=0; i<ABufferSize; i++) {
        b = *ABuffer++;
        //klPrintf("%X ", b);
        I2C_SendData(I2C1, b);
        do {
            IEvt = I2C_GetLastEvent(I2C1);
            if (IEvt & I2C_EVENT_SLAVE_ACK_FAILURE) {   // NACK occured, slave doesn't answer
                klPrintf("\rI2C Slave NACK\r");
                I2C_GenerateSTOP(I2C1, ENABLE);
                return;
            }
        } while(IEvt != I2C_EVENT_MASTER_BYTE_TRANSMITTED);
    } // for
    I2C_GenerateSTOP(I2C1, ENABLE);
    //klPrintf("\rI2C write completed\r");
}
