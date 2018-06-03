/*
 * kl_ini.h
 *
 *  Created on: 03.07.2011
 *      Author: Kreyl
 */

#ifndef KL_INI_H_
#define KL_INI_H_

#include "ff.h"
#include "diskio.h"
#include <inttypes.h>
#include "color.h"

#define INI_BUF_SIZE    512

bool ReadString(const char *ASection, const char *AKey, const char *AFileName, char *AOutput, uint32_t AMaxLen);
bool ReadInt32 (const char *ASection, const char *AKey, const char *AFileName, int32_t *AOutput);
bool ReadUint32(const char *ASection, const char *AKey, const char *AFileName, uint32_t *AOutput);
bool ReadColor (const char *ASection, const char *AKey, const char *AFileName, Color_t *AOutput);

#endif /* KL_INI_H_ */
