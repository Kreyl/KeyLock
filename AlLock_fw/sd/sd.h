/*
 * File:   sd.h
 * Author: Kreyl
 *
 * Created on June 5, 2011, 6:40 PM
 */

#ifndef SD_H
#define	SD_H

#include "stm32f10x.h"
#include "stm32f10x_sdio.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#include "diskio.h"
#include "ff.h"
//#include "stm32_eval_sdio_sd.h"

// ============================ Defines ========================================
#define SD_BUF_SIZE 4096

// Change this if needed
// SDIO Intialization Frequency (400KHz max)
#define SDIO_INIT_CLK_DIV                ((uint8_t)0xB2)
// SDIO Data Transfer Frequency (25MHz max)
#define SDIO_TRANSFER_CLK_DIV            ((uint8_t)4)

// Constants
#define SDIO_FIFO_ADDRESS                ((uint32_t)0x40018080)

// Supported SD Memory Cards
#define SDIO_STD_CAPACITY_SD_CARD_V1_1             ((uint32_t)0x00000000)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0             ((uint32_t)0x00000001)
#define SDIO_HIGH_CAPACITY_SD_CARD                 ((uint32_t)0x00000002)
#define SDIO_MULTIMEDIA_CARD                       ((uint32_t)0x00000003)
#define SDIO_SECURE_DIGITAL_IO_CARD                ((uint32_t)0x00000004)
#define SDIO_HIGH_SPEED_MULTIMEDIA_CARD            ((uint32_t)0x00000005)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD          ((uint32_t)0x00000006)
#define SDIO_HIGH_CAPACITY_MMC_CARD                ((uint32_t)0x00000007)

// ============================= Types =========================================
typedef enum {
// SDIO specific error defines
  SD_CMD_CRC_FAIL                    = (1), /*!< Command response received (but CRC check failed) */
  SD_DATA_CRC_FAIL                   = (2), /*!< Data bock sent/received (CRC check Failed) */
  SD_CMD_RSP_TIMEOUT                 = (3), /*!< Command response timeout */
  SD_DATA_TIMEOUT                    = (4), /*!< Data time out */
  SD_TX_UNDERRUN                     = (5), /*!< Transmit FIFO under-run */
  SD_RX_OVERRUN                      = (6), /*!< Receive FIFO over-run */
  SD_START_BIT_ERR                   = (7), /*!< Start bit not detected on all data signals in widE bus mode */
  SD_CMD_OUT_OF_RANGE                = (8), /*!< CMD's argument was out of range.*/
  SD_ADDR_MISALIGNED                 = (9), /*!< Misaligned address */
  SD_BLOCK_LEN_ERR                   = (10), /*!< Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length */
  SD_ERASE_SEQ_ERR                   = (11), /*!< An error in the sequence of erase command occurs.*/
  SD_BAD_ERASE_PARAM                 = (12), /*!< An Invalid selection for erase groups */
  SD_WRITE_PROT_VIOLATION            = (13), /*!< Attempt to program a write protect block */
  SD_LOCK_UNLOCK_FAILED              = (14), /*!< Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card */
  SD_COM_CRC_FAILED                  = (15), /*!< CRC check of the previous command failed */
  SD_ILLEGAL_CMD                     = (16), /*!< Command is not legal for the card state */
  SD_CARD_ECC_FAILED                 = (17), /*!< Card internal ECC was applied but failed to correct the data */
  SD_CC_ERROR                        = (18), /*!< Internal card controller error */
  SD_GENERAL_UNKNOWN_ERROR           = (19), /*!< General or Unknown error */
  SD_STREAM_READ_UNDERRUN            = (20), /*!< The card could not sustain data transfer in stream read operation. */
  SD_STREAM_WRITE_OVERRUN            = (21), /*!< The card could not sustain data programming in stream mode */
  SD_CID_CSD_OVERWRITE               = (22), /*!< CID/CSD overwrite error */
  SD_WP_ERASE_SKIP                   = (23), /*!< only partial address space was erased */
  SD_CARD_ECC_DISABLED               = (24), /*!< Command has been executed without using internal ECC */
  SD_ERASE_RESET                     = (25), /*!< Erase sequence was cleared before executing because an out of erase sequence command was received */
  SD_AKE_SEQ_ERROR                   = (26), /*!< Error in sequence of authentication. */
  SD_INVALID_VOLTRANGE               = (27),
  SD_ADDR_OUT_OF_RANGE               = (28),
  SD_SWITCH_ERROR                    = (29),
  SD_SDIO_DISABLED                   = (30),
  SD_SDIO_FUNCTION_BUSY              = (31),
  SD_SDIO_FUNCTION_FAILED            = (32),
  SD_SDIO_UNKNOWN_FUNCTION           = (33),

// Standard error defines
  SD_INTERNAL_ERROR                  = (34),
  SD_NOT_CONFIGURED                  = (35),
  SD_REQUEST_PENDING                 = (36),
  SD_REQUEST_NOT_APPLICABLE          = (37),
  SD_INVALID_PARAMETER               = (38),
  SD_UNSUPPORTED_FEATURE             = (39),
  SD_UNSUPPORTED_HW                  = (40),
  SD_ERROR                           = (41),
  SD_OK = 0
} SD_Error;
typedef enum {rspNone, rsp1, rsp2, rsp3, rsp6, rsp7} SD_RespType_t;

// Card Specific Data: CSD Register
typedef struct {
    __IO uint8_t  CSDStruct;            /*!< CSD structure */
    __IO uint8_t  SysSpecVersion;       /*!< System specification version */
    __IO uint8_t  Reserved1;            /*!< Reserved */
    __IO uint8_t  TAAC;                 /*!< Data read access-time 1 */
    __IO uint8_t  NSAC;                 /*!< Data read access-time 2 in CLK cycles */
    __IO uint8_t  MaxBusClkFrec;        /*!< Max. bus clock frequency */
    __IO uint16_t CardComdClasses;      /*!< Card command classes */
    __IO uint8_t  RdBlockLen;           /*!< Max. read data block length */
    __IO uint8_t  PartBlockRead;        /*!< Partial blocks for read allowed */
    __IO uint8_t  WrBlockMisalign;      /*!< Write block misalignment */
    __IO uint8_t  RdBlockMisalign;      /*!< Read block misalignment */
    __IO uint8_t  DSRImpl;              /*!< DSR implemented */
    __IO uint8_t  Reserved2;            /*!< Reserved */
    __IO uint32_t DeviceSize;           /*!< Device Size */
    __IO uint8_t  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min */
    __IO uint8_t  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max */
    __IO uint8_t  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min */
    __IO uint8_t  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max */
    __IO uint8_t  DeviceSizeMul;        /*!< Device size multiplier */
    __IO uint8_t  EraseGrSize;          /*!< Erase group size */
    __IO uint8_t  EraseGrMul;           /*!< Erase group size multiplier */
    __IO uint8_t  WrProtectGrSize;      /*!< Write protect group size */
    __IO uint8_t  WrProtectGrEnable;    /*!< Write protect group enable */
    __IO uint8_t  ManDeflECC;           /*!< Manufacturer default ECC */
    __IO uint8_t  WrSpeedFact;          /*!< Write speed factor */
    __IO uint8_t  MaxWrBlockLen;        /*!< Max. write data block length */
    __IO uint8_t  WriteBlockPaPartial;  /*!< Partial blocks for write allowed */
    __IO uint8_t  Reserved3;            /*!< Reserded */
    __IO uint8_t  ContentProtectAppli;  /*!< Content protection application */
    __IO uint8_t  FileFormatGrouop;     /*!< File format group */
    __IO uint8_t  CopyFlag;             /*!< Copy flag (OTP) */
    __IO uint8_t  PermWrProtect;        /*!< Permanent write protection */
    __IO uint8_t  TempWrProtect;        /*!< Temporary write protection */
    __IO uint8_t  FileFormat;           /*!< File Format */
    __IO uint8_t  ECC;                  /*!< ECC code */
    __IO uint8_t  CSD_CRC;              /*!< CSD CRC */
    __IO uint8_t  Reserved4;            /*!< always 1*/
} SD_CSD;
// Card Identification Data: CID Register
typedef struct {
    __IO uint8_t  ManufacturerID;       /*!< ManufacturerID */
    __IO uint16_t OEM_AppliID;          /*!< OEM/Application ID */
    __IO uint32_t ProdName1;            /*!< Product Name part1 */
    __IO uint8_t  ProdName2;            /*!< Product Name part2*/
    __IO uint8_t  ProdRev;              /*!< Product Revision */
    __IO uint32_t ProdSN;               /*!< Product Serial Number */
    __IO uint8_t  Reserved1;            /*!< Reserved1 */
    __IO uint16_t ManufactDate;         /*!< Manufacturing Date */
    __IO uint8_t  CID_CRC;              /*!< CID CRC */
    __IO uint8_t  Reserved2;            /*!< always 1 */
} SD_CID;

// SD Card information
typedef struct {
    SD_CSD SD_csd;
    SD_CID SD_cid;
    uint32_t CardCapacity;  /*!< Card Capacity */
    uint32_t CardBlockSize; /*!< Card Block Size */
    uint16_t RCA;
    uint8_t CardType;
} SD_CardInfo;

class sd_t {
private:
    // Variables
    SD_CardInfo CardInfo;
    uint32_t CID_Tab[4], CSD_Tab[4];
    SD_Error Cmd(uint32_t AIndx, uint32_t AArgument, SD_RespType_t ARespType, ...);
    // Init
    void LowLevelInit(void);
    void InitSDIO(uint8_t AClkDiv, uint32_t ABusWide);
    SD_Error PowerOn(void);
    SD_Error InitCards(void);
    void GetCardInfo(void);
    SD_Error SelectDeselect(uint32_t AAddr) { return Cmd(7, AAddr, rsp1); }
    SD_Error EnableWideBusOperation(uint32_t WideMode);
    SD_Error SDEnWideBus(FunctionalState NewState);
    SD_Error FindSCR(uint32_t *pscr);
public:
    // Files data
    FATFS FatFilesystem;
    // SD data
    SD_Error Status;
    SD_Error Init(void);
    // Read/write
    SD_Error ReadBlock(uint8_t *ABuf, uint32_t AAddr, uint16_t ABlockSize);
};

extern sd_t SD;


#endif	/* SD_H */

