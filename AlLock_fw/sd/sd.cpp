#include "misc.h"
#include "sd.h"
#include <stdarg.h>


#include "kl_util.h"
#include "ff.h"
#include "delay_util.h"


#define BLOCK_SIZE          512 /* Block Size in Bytes */
#define NUMBER_OF_BLOCKS    4  /* For Multi Blocks operation (Read/Write) */
#define MULTI_BUFFER_SIZE   1024

uint8_t Buffer_Rx[MULTI_BUFFER_SIZE];

// Defines
#define SD_VOLTAGE_WINDOW_SD    ((uint32_t)0x80100000)
#define SD_HIGH_CAPACITY        ((uint32_t)0x40000000)
#define SD_STD_CAPACITY         ((uint32_t)0x00000000)
#define SDIO_STATIC_FLAGS       ((uint32_t)0x000005FF)
#define SD_OCR_ERRORBITS        ((uint32_t)0xFDFFE008)

#define SD_0TO7BITS                     ((uint32_t)0x000000FF)
#define SD_8TO15BITS                    ((uint32_t)0x0000FF00)
#define SD_16TO23BITS                   ((uint32_t)0x00FF0000)
#define SD_24TO31BITS                   ((uint32_t)0xFF000000)


//  Masks for R6 Response
#define SD_R6_GENERAL_UNKNOWN_ERROR     ((uint32_t)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((uint32_t)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((uint32_t)0x00008000)
// Bus width
#define SD_CARD_LOCKED                  ((uint32_t)0x02000000)
#define SD_WIDE_BUS_SUPPORT             ((uint32_t)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((uint32_t)0x00010000)

// Timings
#define SD_CMD_TIMEOUT      10000


// Variables
sd_t SD;

/**
 * Initializes the SD Card and put it into StandBy State (Ready for data transfer).
 * SD_Error: SD Card Error code.
 */
SD_Error sd_t::Init() {
//#define SD_VERBOSE_INIT
    //klPrintf("SD Init\r");
    SD_Error errorstatus = SD_OK;
    CardInfo.CardType = SDIO_STD_CAPACITY_SD_CARD_V1_1;
    LowLevelInit();                         // Initialize clocks, pins, etc.
    InitSDIO(SDIO_INIT_CLK_DIV, SDIO_BusWide_1b);  // Set 400 kHz clk
    SDIO_SetPowerState(SDIO_PowerState_ON); // Set Power State to ON
    SDIO_ClockCmd(ENABLE);                  // Enable SDIO Clock
    errorstatus = PowerOn();                // Try to power on
#ifdef SD_VERBOSE_INIT
    klPrintf("SD PowerOn: %i\r", errorstatus);
#endif
    if (errorstatus != SD_OK) return (errorstatus);
    errorstatus = InitCards();
#ifdef SD_VERBOSE_INIT
    klPrintf("SD_InitCards: %i\r", errorstatus);
#endif
    if (errorstatus != SD_OK) return (errorstatus);
    // Speed-up SDIO
    InitSDIO(SDIO_TRANSFER_CLK_DIV, SDIO_BusWide_1b);
    GetCardInfo();
#ifdef SD_VERBOSE_INIT
    // Print card info
    klPrintf("SD Info:\r");
    klPrintf("CardBlockSize: %u\r", CardInfo.CardBlockSize);
    klPrintf("CardCapacity: %u\r", CardInfo.CardCapacity);
    klPrintf("CardType: %u\r", CardInfo.Type);
    klPrintf("RCA: %u\r", CardInfo.RCA);
#endif
    // Select Card
    errorstatus = SelectDeselect((uint32_t) (CardInfo.RCA << 16));
#ifdef SD_VERBOSE_INIT
    klPrintf("SelectDeselect: %i\r", errorstatus);
#endif
    if (errorstatus != SD_OK) return (errorstatus);
    errorstatus = EnableWideBusOperation(SDIO_BusWide_4b);
#ifdef SD_VERBOSE_INIT
    klPrintf("EnableWideBusOperation: %i\r", errorstatus);
#endif
    if (errorstatus != SD_OK) return (errorstatus);

    // KL: Send block size
    errorstatus = Cmd(16, (uint32_t)512, rsp1);
#ifdef SD_VERBOSE_INIT
    klPrintf("SD_CMD_SET_BLOCKLEN: %u\r", errorstatus);
#endif
    return (errorstatus);
//            Status = SD_ReadBlock(Buffer_Rx, (0x0083 * 512), 512);
//            if (Status == SD_OK) {
//                for (uint32_t i = 0; i < 512; i++) {
//                    UART_PrintAsHex(Buffer_Rx[i]);
//                    UART_Print(' ');
//                }
//                UART_NewLine();
//            }
//            else UART_PrintString("err");
}

void sd_t::LowLevelInit(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    // GPIOC and GPIOD Periph clock enable
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
    // Configure PC.08, PC.09, PC.10, PC.11, PC.12 pin: D0, D1, D2, D3, CLK pin
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    // Configure PD.02 CMD line
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // Enable the SDIO AHB Clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SDIO, ENABLE);
    // Enable the DMA2 Clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
    SDIO_DeInit();
}

void sd_t::InitSDIO(uint8_t AClkDiv, uint32_t ABusWide) {
    /*!< Configure the SDIO peripheral */
    /*!< SDIOCLK = HCLK, SDIO_CK = HCLK/(2 + SDIO_INIT_CLK_DIV) */
    /*!< on STM32F2xx devices, SDIOCLK is fixed to 48MHz */
    /*!< SDIO_CK for initialization should not exceed 400 KHz */
    SDIO_InitTypeDef SDIO_InitStructure;
    SDIO_InitStructure.SDIO_ClockDiv = AClkDiv;
    SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
    SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
    SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
    SDIO_InitStructure.SDIO_BusWide = ABusWide;
    SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
    //SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Enable;
    SDIO_Init(&SDIO_InitStructure);
}

SD_Error sd_t::PowerOn() {
//#define SD_VERBOSE_POWER_ON
#ifdef SD_VERBOSE_POWER_ON
    klPrintf("SD Power on\r");
#endif
    SD_Error errorstatus = SD_OK;
    uint32_t SDType = SD_STD_CAPACITY;
    // CMD0: GO_IDLE_STATE
    errorstatus = Cmd(0, 0, rspNone);
    if (errorstatus != SD_OK) return (errorstatus);

    // Send CMD8 to verify SD card interface operating condition.
    // Argument: - [31:12]: Reserved (shall be set to '0') - [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V) - [7:0]: Check Pattern (recommended 0xAA) */
    errorstatus = Cmd(8, (uint32_t)0x1AA, rsp7);
    if (errorstatus == SD_OK) {
        CardInfo.CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0;
        SDType = SD_HIGH_CAPACITY;  // needed later
    } else errorstatus = Cmd(55, 0, rsp1);

    errorstatus = Cmd(55, 0, rsp1);
    // If errorstatus is Command TimeOut, it is a MMC card.
    // If errorstatus is SD_OK it is a SD card: SD card 2.0 (voltage range mismatch) or SD card 1.x
    if (errorstatus == SD_OK) { // SD CARD
        // Send ACMD41 SD_APP_OP_COND with Argument 0x80100000
        uint32_t counter = 0, response, validvoltage = 0;
        do {
#ifdef SD_VERBOSE_POWER_ON
            klPrintf("Iteration: %u\r", counter);
#endif
            errorstatus = Cmd(55, 0, rsp1);
            if (errorstatus != SD_OK) return (errorstatus);
            errorstatus = Cmd(41, (SD_VOLTAGE_WINDOW_SD | SDType), rsp3);
            if (errorstatus != SD_OK) return (errorstatus);

            response = SDIO_GetResponse(SDIO_RESP1);
            validvoltage = (((response >> 31) == 1) ? 1 : 0);
#ifdef SD_VERBOSE_POWER_ON
            klPrintf("Response: %u   Voltage: %u\r", response, validvoltage);
#endif
            if (counter++ == 0xFFFF) return SD_INVALID_VOLTRANGE;
        } while (!validvoltage);
        if (response &= SD_HIGH_CAPACITY) CardInfo.CardType = SDIO_HIGH_CAPACITY_SD_CARD;
    } // if SD_OK; else this is MMC card
    return (SD_OK);
}

SD_Error sd_t::InitCards() {
    //klPrintf("InitCards\r");
    if (SDIO_GetPowerState() == SDIO_PowerState_OFF) return SD_REQUEST_NOT_APPLICABLE;
    SD_Error errorstatus = SD_OK;

    if (CardInfo.CardType != SDIO_SECURE_DIGITAL_IO_CARD) {
        // Send CMD2: ALL_SEND_CID
        errorstatus = Cmd(2, 0, rsp2);
        if (errorstatus != SD_OK) return (errorstatus);
        CID_Tab[0] = SDIO_GetResponse(SDIO_RESP1);
        CID_Tab[1] = SDIO_GetResponse(SDIO_RESP2);
        CID_Tab[2] = SDIO_GetResponse(SDIO_RESP3);
        CID_Tab[3] = SDIO_GetResponse(SDIO_RESP4);
    }

    if (    (CardInfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) ||
            (CardInfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0) ||
            (CardInfo.CardType == SDIO_SECURE_DIGITAL_IO_COMBO_CARD) ||
            (CardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD)
        )
    {
        // Send CMD3 = SET_REL_ADDR with argument 0. SD Card publishes its RCA.
        errorstatus = Cmd(3, 0, rsp6, &CardInfo.RCA);
        if (errorstatus != SD_OK) return (errorstatus);
    }

    if (CardInfo.CardType != SDIO_SECURE_DIGITAL_IO_CARD) {
        // Send CMD9 SEND_CSD with argument as card's RCA
        errorstatus = Cmd(9, (uint32_t)(CardInfo.RCA << 16), rsp2);
        if (errorstatus != SD_OK) return (errorstatus);
        CSD_Tab[0] = SDIO_GetResponse(SDIO_RESP1);
        CSD_Tab[1] = SDIO_GetResponse(SDIO_RESP2);
        CSD_Tab[2] = SDIO_GetResponse(SDIO_RESP3);
        CSD_Tab[3] = SDIO_GetResponse(SDIO_RESP4);
    }
    return errorstatus;
}

void sd_t::GetCardInfo(void) {
    uint8_t tmp = 0;
    /*!< Byte 0 */
    tmp = (uint8_t) ((CSD_Tab[0] & 0xFF000000) >> 24);
    CardInfo.SD_csd.CSDStruct = (tmp & 0xC0) >> 6;
    CardInfo.SD_csd.SysSpecVersion = (tmp & 0x3C) >> 2;
    CardInfo.SD_csd.Reserved1 = tmp & 0x03;
    /*!< Byte 1 */
    tmp = (uint8_t) ((CSD_Tab[0] & 0x00FF0000) >> 16);
    CardInfo.SD_csd.TAAC = tmp;
    /*!< Byte 2 */
    tmp = (uint8_t) ((CSD_Tab[0] & 0x0000FF00) >> 8);
    CardInfo.SD_csd.NSAC = tmp;
    /*!< Byte 3 */
    tmp = (uint8_t) (CSD_Tab[0] & 0x000000FF);
    CardInfo.SD_csd.MaxBusClkFrec = tmp;
    /*!< Byte 4 */
    tmp = (uint8_t) ((CSD_Tab[1] & 0xFF000000) >> 24);
    CardInfo.SD_csd.CardComdClasses = tmp << 4;
    /*!< Byte 5 */
    tmp = (uint8_t) ((CSD_Tab[1] & 0x00FF0000) >> 16);
    CardInfo.SD_csd.CardComdClasses |= (tmp & 0xF0) >> 4;
    CardInfo.SD_csd.RdBlockLen = tmp & 0x0F;
    /*!< Byte 6 */
    tmp = (uint8_t) ((CSD_Tab[1] & 0x0000FF00) >> 8);
    CardInfo.SD_csd.PartBlockRead = (tmp & 0x80) >> 7;
    CardInfo.SD_csd.WrBlockMisalign = (tmp & 0x40) >> 6;
    CardInfo.SD_csd.RdBlockMisalign = (tmp & 0x20) >> 5;
    CardInfo.SD_csd.DSRImpl = (tmp & 0x10) >> 4;
    CardInfo.SD_csd.Reserved2 = 0; /*!< Reserved */

    if ((CardInfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (CardInfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0)) {
        CardInfo.SD_csd.DeviceSize = (tmp & 0x03) << 10;
        /*!< Byte 7 */
        tmp = (uint8_t) (CSD_Tab[1] & 0x000000FF);
        CardInfo.SD_csd.DeviceSize |= (tmp) << 2;
        /*!< Byte 8 */
        tmp = (uint8_t) ((CSD_Tab[2] & 0xFF000000) >> 24);
        CardInfo.SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;
        CardInfo.SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
        CardInfo.SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);
        /*!< Byte 9 */
        tmp = (uint8_t) ((CSD_Tab[2] & 0x00FF0000) >> 16);
        CardInfo.SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
        CardInfo.SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
        CardInfo.SD_csd.DeviceSizeMul = (tmp & 0x03) << 1;
        /*!< Byte 10 */
        tmp = (uint8_t) ((CSD_Tab[2] & 0x0000FF00) >> 8);
        CardInfo.SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;
        CardInfo.CardCapacity = (CardInfo.SD_csd.DeviceSize + 1);
        CardInfo.CardCapacity *= (1 << (CardInfo.SD_csd.DeviceSizeMul + 2));
        CardInfo.CardBlockSize = 1 << (CardInfo.SD_csd.RdBlockLen);
        CardInfo.CardCapacity *= CardInfo.CardBlockSize;
    } else if (CardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD) {
        /*!< Byte 7 */
        tmp = (uint8_t) (CSD_Tab[1] & 0x000000FF);
        CardInfo.SD_csd.DeviceSize = (tmp & 0x3F) << 16;
        /*!< Byte 8 */
        tmp = (uint8_t) ((CSD_Tab[2] & 0xFF000000) >> 24);
        CardInfo.SD_csd.DeviceSize |= (tmp << 8);
        /*!< Byte 9 */
        tmp = (uint8_t) ((CSD_Tab[2] & 0x00FF0000) >> 16);
        CardInfo.SD_csd.DeviceSize |= (tmp);
        /*!< Byte 10 */
        tmp = (uint8_t) ((CSD_Tab[2] & 0x0000FF00) >> 8);
        CardInfo.CardCapacity = (CardInfo.SD_csd.DeviceSize + 1) * 512 * 1024;
        CardInfo.CardBlockSize = 512;
    }

    CardInfo.SD_csd.EraseGrSize = (tmp & 0x40) >> 6;
    CardInfo.SD_csd.EraseGrMul = (tmp & 0x3F) << 1;
    /*!< Byte 11 */
    tmp = (uint8_t) (CSD_Tab[2] & 0x000000FF);
    CardInfo.SD_csd.EraseGrMul |= (tmp & 0x80) >> 7;
    CardInfo.SD_csd.WrProtectGrSize = (tmp & 0x7F);
    /*!< Byte 12 */
    tmp = (uint8_t) ((CSD_Tab[3] & 0xFF000000) >> 24);
    CardInfo.SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7;
    CardInfo.SD_csd.ManDeflECC = (tmp & 0x60) >> 5;
    CardInfo.SD_csd.WrSpeedFact = (tmp & 0x1C) >> 2;
    CardInfo.SD_csd.MaxWrBlockLen = (tmp & 0x03) << 2;
    /*!< Byte 13 */
    tmp = (uint8_t) ((CSD_Tab[3] & 0x00FF0000) >> 16);
    CardInfo.SD_csd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
    CardInfo.SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
    CardInfo.SD_csd.Reserved3 = 0;
    CardInfo.SD_csd.ContentProtectAppli = (tmp & 0x01);
    /*!< Byte 14 */
    tmp = (uint8_t) ((CSD_Tab[3] & 0x0000FF00) >> 8);
    CardInfo.SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7;
    CardInfo.SD_csd.CopyFlag = (tmp & 0x40) >> 6;
    CardInfo.SD_csd.PermWrProtect = (tmp & 0x20) >> 5;
    CardInfo.SD_csd.TempWrProtect = (tmp & 0x10) >> 4;
    CardInfo.SD_csd.FileFormat = (tmp & 0x0C) >> 2;
    CardInfo.SD_csd.ECC = (tmp & 0x03);
    /*!< Byte 15 */
    tmp = (uint8_t) (CSD_Tab[3] & 0x000000FF);
    CardInfo.SD_csd.CSD_CRC = (tmp & 0xFE) >> 1;
    CardInfo.SD_csd.Reserved4 = 1;

    /*!< Byte 0 */
    tmp = (uint8_t) ((CID_Tab[0] & 0xFF000000) >> 24);
    CardInfo.SD_cid.ManufacturerID = tmp;
    /*!< Byte 1 */
    tmp = (uint8_t) ((CID_Tab[0] & 0x00FF0000) >> 16);
    CardInfo.SD_cid.OEM_AppliID = tmp << 8;
    /*!< Byte 2 */
    tmp = (uint8_t) ((CID_Tab[0] & 0x000000FF00) >> 8);
    CardInfo.SD_cid.OEM_AppliID |= tmp;
    /*!< Byte 3 */
    tmp = (uint8_t) (CID_Tab[0] & 0x000000FF);
    CardInfo.SD_cid.ProdName1 = tmp << 24;
    /*!< Byte 4 */
    tmp = (uint8_t) ((CID_Tab[1] & 0xFF000000) >> 24);
    CardInfo.SD_cid.ProdName1 |= tmp << 16;
    /*!< Byte 5 */
    tmp = (uint8_t) ((CID_Tab[1] & 0x00FF0000) >> 16);
    CardInfo.SD_cid.ProdName1 |= tmp << 8;
    /*!< Byte 6 */
    tmp = (uint8_t) ((CID_Tab[1] & 0x0000FF00) >> 8);
    CardInfo.SD_cid.ProdName1 |= tmp;
    /*!< Byte 7 */
    tmp = (uint8_t) (CID_Tab[1] & 0x000000FF);
    CardInfo.SD_cid.ProdName2 = tmp;
    /*!< Byte 8 */
    tmp = (uint8_t) ((CID_Tab[2] & 0xFF000000) >> 24);
    CardInfo.SD_cid.ProdRev = tmp;
    /*!< Byte 9 */
    tmp = (uint8_t) ((CID_Tab[2] & 0x00FF0000) >> 16);
    CardInfo.SD_cid.ProdSN = tmp << 24;
    /*!< Byte 10 */
    tmp = (uint8_t) ((CID_Tab[2] & 0x0000FF00) >> 8);
    CardInfo.SD_cid.ProdSN |= tmp << 16;
    /*!< Byte 11 */
    tmp = (uint8_t) (CID_Tab[2] & 0x000000FF);
    CardInfo.SD_cid.ProdSN |= tmp << 8;
    /*!< Byte 12 */
    tmp = (uint8_t) ((CID_Tab[3] & 0xFF000000) >> 24);
    CardInfo.SD_cid.ProdSN |= tmp;
    /*!< Byte 13 */
    tmp = (uint8_t) ((CID_Tab[3] & 0x00FF0000) >> 16);
    CardInfo.SD_cid.Reserved1 |= (tmp & 0xF0) >> 4;
    CardInfo.SD_cid.ManufactDate = (tmp & 0x0F) << 8;
    /*!< Byte 14 */
    tmp = (uint8_t) ((CID_Tab[3] & 0x0000FF00) >> 8);
    CardInfo.SD_cid.ManufactDate |= tmp;
    /*!< Byte 15 */
    tmp = (uint8_t) (CID_Tab[3] & 0x000000FF);
    CardInfo.SD_cid.CID_CRC = (tmp & 0xFE) >> 1;
    CardInfo.SD_cid.Reserved2 = 1;
}

/* Find the SD card SCR register value.
* pscr: pointer to the buffer that will contain the SCR value.
*/
SD_Error sd_t::FindSCR(uint32_t *pscr) {
   uint32_t i = 0, tmp;
   SD_Error errorstatus = SD_OK;
   uint32_t tempscr[2] = {0, 0};

   // Set Block Size To 8 Bytes
   errorstatus = Cmd(16, 8, rsp1);
   if (errorstatus != SD_OK) return (errorstatus);
   // Send CMD55 APP_CMD with argument as card's RCA
   errorstatus = Cmd(55, ((uint32_t) CardInfo.RCA << 16), rsp1);
   if (errorstatus != SD_OK) return (errorstatus);

   SDIO_DataInitTypeDef SDIO_DataInitStructure;
   SDIO_DataInitStructure.SDIO_DataTimeOut = 0xFFFFFFFF;
   SDIO_DataInitStructure.SDIO_DataLength = 8;
   SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_8b;
   SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
   SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
   SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
   SDIO_DataConfig(&SDIO_DataInitStructure);

   // Send ACMD51 SD_APP_SEND_SCR with argument as 0
   errorstatus = Cmd(51, 0, rsp1);
   if (errorstatus != SD_OK) return (errorstatus);

   while (!(SDIO->STA & (SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR))) {
       if (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) {
           tmp = SDIO_ReadData();
           if ((i == 0) || (i == 1)) tempscr[i] = tmp;
           i++;
       }
   }

   if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) {
       SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
       return SD_DATA_TIMEOUT;
   } else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) {
       SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
       return SD_DATA_CRC_FAIL;
   } else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET) {
       SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
       return SD_RX_OVERRUN;
   } else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) {
       SDIO_ClearFlag(SDIO_FLAG_STBITERR);
       return SD_START_BIT_ERR;
   }
   // Receive succeded
   SDIO_ClearFlag(SDIO_STATIC_FLAGS);
   *(pscr + 1) = ((tempscr[0] & SD_0TO7BITS) << 24) | ((tempscr[0] & SD_8TO15BITS) << 8) | ((tempscr[0] & SD_16TO23BITS) >> 8) | ((tempscr[0] & SD_24TO31BITS) >> 24);
   *(pscr) = ((tempscr[1] & SD_0TO7BITS) << 24) | ((tempscr[1] & SD_8TO15BITS) << 8) | ((tempscr[1] & SD_16TO23BITS) >> 8) | ((tempscr[1] & SD_24TO31BITS) >> 24);

   return SD_OK;
}

SD_Error sd_t::SDEnWideBus(FunctionalState NewState) {
    if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED) return SD_LOCK_UNLOCK_FAILED;
    uint32_t scr[2];
    SD_Error errorstatus = FindSCR(scr); // Get SCR Register
    if (errorstatus != SD_OK) return (errorstatus);
    // Set wideness
    if (NewState == ENABLE) {
        // If requested card supports wide bus operation
        if ((scr[1] & SD_WIDE_BUS_SUPPORT) != 0) {
            // Send CMD55 APP_CMD with argument as card's RCA
            errorstatus = Cmd(55, ((uint32_t) CardInfo.RCA << 16), rsp1);
            if (errorstatus != SD_OK) return (errorstatus);
            // Send ACMD6 APP_CMD with argument as 2 for wide bus mode
            errorstatus = Cmd(6, 2, rsp1);
            return (errorstatus);
        } else return SD_REQUEST_NOT_APPLICABLE;
    }
    else { // Disable
        // If requested card supports 1 bit mode operation
        if ((scr[1] & SD_SINGLE_BUS_SUPPORT) != 0) {
            // Send CMD55 APP_CMD with argument as card's RCA
            errorstatus = Cmd(55, ((uint32_t) CardInfo.RCA << 16), rsp1);
            if (errorstatus != SD_OK) return (errorstatus);
            // Send ACMD6 APP_CMD with argument as 0 for 1bit bus mode
            errorstatus = Cmd(6, 0, rsp1);
            return (errorstatus);
        } else return SD_REQUEST_NOT_APPLICABLE;
    }
} // SDEnWideBus

/**
 * Enables wide bus opeartion for the requeseted card if supported by card.
 * WideMode: Specifies the SD card wide bus mode.
 *   This parameter can be one of the following values:
 *     SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
 *     SDIO_BusWide_4b: 4-bit data transfer
 *     SDIO_BusWide_1b: 1-bit data transfer
 */
SD_Error sd_t::EnableWideBusOperation(uint32_t WideMode) {
    SD_Error errorstatus = SD_OK;
    // MMC Card doesn't support this feature
    if (CardInfo.CardType == SDIO_MULTIMEDIA_CARD) return SD_UNSUPPORTED_FEATURE;
    else if (
            (CardInfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) ||
            (CardInfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0) ||
            (CardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD)) {
        if (WideMode == SDIO_BusWide_8b) return SD_UNSUPPORTED_FEATURE;
        else if (WideMode == SDIO_BusWide_4b) {
            errorstatus = SDEnWideBus(ENABLE);
            if (errorstatus == SD_OK) InitSDIO(SDIO_TRANSFER_CLK_DIV, SDIO_BusWide_4b);
        } else {    // 1bit
            errorstatus = SDEnWideBus(DISABLE);
            if (errorstatus == SD_OK) InitSDIO(SDIO_TRANSFER_CLK_DIV, SDIO_BusWide_1b);
        }
    }
    return (errorstatus);
}

// ========================== Commands and responses ==========================
SD_Error sd_t::Cmd(uint32_t AIndx, uint32_t AArgument, SD_RespType_t ARespType, ...) {
    // Handle arguments
    va_list Arg;
    va_start(Arg, ARespType);    // Set pointer to last argument
    // Setup cmd structure
    SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
    switch (ARespType) {
        case rspNone:
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;
            break;
        case rsp1:
        case rsp3:
        case rsp6:
        case rsp7:
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            break;
        case rsp2:
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
            break;
    } // switch
    // Send cmd
    SDIO_CmdInitStructure.SDIO_Argument = AArgument;
    SDIO_CmdInitStructure.SDIO_CmdIndex = AIndx;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    // Get response
    uint32_t status;
    SD_Error errorstatus = SD_CMD_RSP_TIMEOUT;
    for (uint32_t i=0; i<SD_CMD_TIMEOUT; i++) {
        status = SDIO->STA;
        if ((ARespType == rspNone) && (status & SDIO_FLAG_CMDSENT)) {
            errorstatus = SD_OK;
            break;
        }

        if ((ARespType == rsp1) && (status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT))) {
            errorstatus = (SD_Error) (SDIO->RESP1 & SD_OCR_ERRORBITS);
            break;
        }

        if (status & SDIO_FLAG_CCRCFAIL) {
            if((ARespType == rsp3) || (ARespType == rsp7)) errorstatus = SD_OK;
            else errorstatus = SD_CMD_CRC_FAIL; // rsp2, rsp6
            break;
        }

        if (status & SDIO_FLAG_CMDREND) {
            errorstatus = SD_OK;
            if (ARespType == rsp6) {
                if (SDIO_GetCommandResponse() == AIndx) {
                    uint32_t response_r1 = SDIO_GetResponse(SDIO_RESP1);
                    if ((response_r1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)) == 0)
                        *(va_arg(Arg, uint16_t*)) = (uint16_t) (response_r1 >> 16);

                }
                else errorstatus = SD_ILLEGAL_CMD;
            }
            break;
        }

        if (status & SDIO_FLAG_CTIMEOUT) break;
    } // for
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    //klPrintf("Cmd%u Arg %X: %u\r", AIndx, AArgument, errorstatus);
    va_end(Arg);    // finish working with arguments
    return errorstatus;
}

// =============================== Read/Write =================================
SD_Error sd_t::ReadBlock(uint8_t *ABuf, uint32_t AAddr, uint16_t ABlockSize) {
    SD_Error errorstatus = SD_OK;
    SDIO->DCTRL = 0x0;

    if (CardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD) {
        ABlockSize = 512;
        AAddr /= 512;
    }
    SDIO_DataInitTypeDef SDIO_DataInitStructure;
    SDIO_DataInitStructure.SDIO_DataTimeOut = 0xFFFFFFFF;;
    SDIO_DataInitStructure.SDIO_DataLength = (uint32_t)ABlockSize;
    SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) 9 << 4;  // 512 bytes
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataConfig(&SDIO_DataInitStructure);

    // Send CMD17: READ_SINGLE_BLOCK
    errorstatus = Cmd(17, (uint32_t)AAddr, rsp1);
    if (errorstatus != SD_OK) return (errorstatus);

    SDIO_DMACmd(ENABLE);
    // Init DMA
    DMA_InitTypeDef DMA_InitStructure;
    DMA_ClearFlag(DMA2_FLAG_TC4 | DMA2_FLAG_TE4 | DMA2_FLAG_HT4 | DMA2_FLAG_GL4);
    // DMA2 Channel4 disable
    DMA_Cmd(DMA2_Channel4, DISABLE);
    // DMA2 Channel4 Config
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) SDIO_FIFO_ADDRESS;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) ABuf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = ABlockSize / 4;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel4, &DMA_InitStructure);
    // DMA2 Channel4 enable
    DMA_Cmd(DMA2_Channel4, ENABLE);

    return SD_OK;
}


// ========================= FAT needed functions ==============================
// Physical drive number (0), Pointer to the data buffer to store read data, Start sector number (LBA), Sector count (1..255)
DRESULT disk_read (BYTE drv, BYTE *buff, DWORD sector, BYTE count) {
	if (drv || !count) return RES_PARERR;
    SD.Status = SD.ReadBlock(buff, (sector * 512), 512);
    if (SD.Status != SD_OK) {
        klPrintf("Err1: %u\r", SD.Status);
        return RES_ERROR;
    }
    // Wait for DMA transfer to finish
    uint32_t t=0;
    while (DMA_GetFlagStatus(DMA2_FLAG_TC4) == RESET) if(t++ > 10000) return RES_ERROR;
    return RES_OK;
}
DSTATUS disk_status (BYTE drv) {
    if (drv) return STA_NOINIT;			// Supports only single drive
    return SD.Status;
}
DSTATUS disk_initialize (BYTE drv) {
    if (drv) return STA_NOINIT;			// Supports only single drive
    SD.Status = SD.Init();
    if (SD.Status != SD_OK) {
        klPrintf("SD init error: %u\r", SD.Status);
        return STA_NOINIT;
    }
    klPrintf("Crd on\r");
    return 0;
}
