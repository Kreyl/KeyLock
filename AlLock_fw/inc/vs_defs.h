/*
 * File:   vs_defs.h
 * Author: Laurelindo
 *
 * Created on 6 Октябрь 2009 г., 0:45
 */

#ifndef _VS1003DEFS_H
#define	_VS1003DEFS_H

// Command codes
#define VS_READ_OPCODE  0b00000011
#define VS_WRITE_OPCODE 0b00000010

// ==== Error codes ====
#define VS_OK           0
#define VS_BUSY_ER      1
#define VS_TIMEOUT_ER   2

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

// Mode flags
#define VS_SM_DIFF      0x0001
#define VS_SM_RESET     0x0004
#define VS_SM_PDOWN     0x0010
#define VS_SM_TESTS     0x0020
#define VS_ICONF_ADSDI  0x0040  // SCI midi, SDI ADPCM
#define VS_SM_SDINEW    0x0800  // native SPI modes; must have

// Mixer volume
#define VS_SMV_ACTIVE   0x8000  // Mixer is active
#define VS_SMV_GAIN2    (25 << 5)     // Gain2 = 0 db, others muted

// VS_REG_RECCTRL
#define VS_SARC_DREQ512     0x0100  // DREQ needs 512 byte space to turn on
#define VS_SARC_OUTOFADPCM  0x0080  // current ADPCM playback is canceled

#endif	/* _VS1003DEFS_H */

