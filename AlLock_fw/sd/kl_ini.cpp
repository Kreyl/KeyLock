/*
 * kl_ini.cpp
 *
 *  Created on: 03.07.2011
 *      Author: Kreyl
 */

#include "kl_ini.h"
#include <string.h>
#include <stdlib.h>

#include "kl_util.h"

char IBuf[INI_BUF_SIZE];

char *skipleading(char *S);
char *skiptrailing(char *S, const char *base);
char *striptrailing(char *S);

// ======================= Implementation =====================================
bool ReadString(const char *ASection, const char *AKey, const char *AFileName, char *AOutput, uint32_t AMaxLen) {
    FRESULT rslt;
    FIL IFile;
    // Open file
    rslt = f_open(&IFile, AFileName, FA_READ+FA_OPEN_EXISTING);
    if (rslt != FR_OK) {
        //klPrintf(AFileName);
        if (rslt == FR_NO_FILE) klPrintf(": file not found\r");
        else klPrintf(": openFile error: %u", rslt);
        return false;
    }
    // Check if zero file
    if (IFile.fsize == 0) {
        f_close(&IFile);
        klPrintf("Empty file\r");
        return false;
    }

    // Move through file one line at a time until a section is matched or EOF.
    char *StartP, *EndP;
    uint32_t len = strlen(ASection);
    do {
        if (!f_gets(IBuf, INI_BUF_SIZE, &IFile)) {
            f_close(&IFile);
            return false;
        }
        StartP = skipleading(IBuf);
        EndP = strchr(StartP, ']');
    } while ((*StartP != '[') || (EndP == NULL) || ((int)(EndP-StartP-1) != len || strnicmp(StartP+1, ASection, len) != 0));

    // Section found, find the key
    len = strlen(AKey);
    do {
        if (!f_gets(IBuf, INI_BUF_SIZE, &IFile) || *(StartP = skipleading(IBuf)) == '[') {
            f_close(&IFile);
            return false;
        }
        StartP = skipleading(IBuf);
        EndP = strchr(StartP, '='); /* Parse out the equal sign */
    } while ((*StartP == ';') || (*StartP == '#') || (EndP == NULL) || ((int)(skiptrailing(EndP, StartP)-StartP) != len || strnicmp(StartP, AKey, len) != 0));

    // Copy up to ALength chars to AOutput
    StartP = skipleading(EndP + 1);
    // Remove a trailing comment
    uint8_t isstring = 0;
    for (EndP = StartP; *EndP != '\0' && ((*EndP != ';' && *EndP != '#') || isstring) && ((EndP - StartP) < AMaxLen); EndP++) {
        if (*EndP == '"') {
            if (*(EndP + 1) == '"') EndP++;     // skip "" (both quotes)
            else isstring = !isstring; // single quote, toggle isstring
        }
        else if (*EndP == '\\' && *(EndP + 1) == '"') EndP++; // skip \" (both quotes)
    } // for
    *EndP = '\0';   // Terminate at a comment
    striptrailing(StartP);
    strcpy(AOutput, StartP);
    f_close(&IFile);
    return &AOutput[EndP-StartP-2]; // Pointer to last '\0'
}

bool ReadInt32 (const char *ASection, const char *AKey, const char *AFileName, int32_t *AOutput) {
    char FBuf[64];
    if (ReadString(ASection, AKey, AFileName, FBuf, 64)) {
        *AOutput = strtol(FBuf, NULL, 10);
        return true;
    }
    else return false;
}
bool ReadUint32(const char *ASection, const char *AKey, const char *AFileName, uint32_t *AOutput) {
    char FBuf[64];
    if (ReadString(ASection, AKey, AFileName, FBuf, 64)) {
        *AOutput = strtol(FBuf, NULL, 10);
        return true;
    }
    else return false;
}

bool ReadColor (const char *ASection, const char *AKey, const char *AFileName, Color_t *AOutput) {
    char FBuf[64];
    if (ReadString(ASection, AKey, AFileName, FBuf, 64)) {
        if (strlen(FBuf) != 6) return false;
        uint32_t N=0;
        if (!HexToUint(&FBuf[0], 2, &N)) return false;
        AOutput->Red = N;
        if (!HexToUint(&FBuf[2], 2, &N)) return false;
        AOutput->Green = N;
        if (!HexToUint(&FBuf[4], 2, &N)) return false;
        AOutput->Blue = N;
        return true;
    }
    else return false;
}

// ============================== Inner use ====================================
char* skipleading(char *S) {
    while (*S != '\0' && *S <= ' ') S++;
    return (char *)S;
}
char* skiptrailing(char *S, const char *base) {
    while ((S > base) && (*(S-1) <= ' ')) S--;
    return (char *)S;
}
char* striptrailing(char *S) {
    char *ptr = skiptrailing(strchr(S, '\0'), S);
    *ptr='\0';
    return S;
}

