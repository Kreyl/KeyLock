/*
 * kl_util.c
 *
 *  Created on: 21.11.2011
 *      Author: Kreyl
 */

#include "kl_util.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include <stdarg.h>
#include <stdbool.h>

/*
 * Converts AValue to NULL-terminated string S.
 * Returns pointer to NULL-terminating symbol.
 */
char *UintToStr(uint32_t AValue, char* S) {
    char digit = '0';
    bool ShouldPrint = false;
    const uint32_t m[9] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10};

    for(uint32_t i=0; i<9; i++) {
        while (AValue >= m[i]) {
            digit++;
            AValue -= m[i];
        }
        if (digit != '0' || ShouldPrint) {
            *S++ = digit;
            ShouldPrint = true;
        }
        digit = '0';
    } // for
    *S++ = ((uint8_t)('0'+AValue));
    *S = 0; // trailing \0
    return S;
}


// ================================ Uart ======================================
char UintToHexChar (uint8_t b);
void UART_PrintAsHex (uint32_t ANumber);
void UART_PrintUint (uint32_t ANumber);
void UART_PrintInt (int32_t ANumber);
void UART_PrintString (const char *S);
void UART_Print8HexArray(uint8_t *Arr, uint32_t ALength);

// ************** Implementation ***********************
void UART_Init(void) {
    // ==== Clocks init ====
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);      // UART clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    // ==== GPIO init ====
    GPIO_InitTypeDef GPIO_InitStructure;
    // Configure USART Tx as alternate function push-pull
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // ==== USART configuration ====
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 128000;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    // Enable USART
    USART_Cmd(USART1, ENABLE);
}
void UART_Print (uint8_t AByte) {
    // Write a character to the USART
    USART_SendData(USART1, AByte);
    // Loop until the end of transmission
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

/*
 * c: character
 * i: signed int
 * u: unsigned int
 * s, S: string
 * X: hexadecimal, capital letters
 * (1...9) width - Minimum number of characters to be printed
 * H - array as 8-bit hexadecimal, length is to be next after uint8_t*
 */
void klPrintf(const char *S, ...) {
    char c;
    bool WasPercent = false;
    va_list Arg;
    va_start(Arg, S);    // Set pointer to first argument
    while ((c = *S) != 0) {
        if (c == '%') {
            if (WasPercent) {
                UART_Print(c);  // %%
                WasPercent = false;
            }
            else WasPercent = true;
        }
        else { // not %
            if (WasPercent) {
                if (c == 'c') UART_Print((uint8_t)va_arg(Arg, int32_t));
                else if (c == 'u') UART_PrintUint(va_arg(Arg, uint32_t));
                else if (c == 'i') UART_PrintInt(va_arg(Arg, int32_t));
                else if (c == 'X') UART_PrintAsHex(va_arg(Arg, uint32_t));
                else if ((c == 's') || (c == 'S')) UART_PrintString(va_arg(Arg, char*));
                else if (c == 'H') UART_Print8HexArray(va_arg(Arg, uint8_t*), va_arg(Arg, uint32_t));
                WasPercent = false;
            } // if was percent
            else UART_Print(c);
        }
        S++;
    } // while
    va_end(Arg);
}

void UART_PrintAsHex (uint32_t ANumber) {
    bool MustPrint = false;
    if (ANumber & 0xFF000000) {
        UART_Print(UintToHexChar (0x0F & (ANumber >> 28)));
        UART_Print(UintToHexChar (0x0F & (ANumber >> 24)));
        MustPrint = true;
    }
    if ((ANumber & 0x00FF0000) || MustPrint) {
        UART_Print(UintToHexChar (0x0F & (ANumber >> 20)));
        UART_Print(UintToHexChar (0x0F & (ANumber >> 16)));
        MustPrint = true;
    }
    if ((ANumber & 0x0000FF00) || MustPrint) {
        UART_Print(UintToHexChar (0x0F & (ANumber >> 12)));
        UART_Print(UintToHexChar (0x0F & (ANumber >> 8)));
    }
    UART_Print(UintToHexChar (0x0F & (ANumber >> 4)));
    UART_Print(UintToHexChar (0x0F & (ANumber)));
}

// ==== Simple conversions ====
char UintToHexChar (uint8_t b) {
    return ((b<=0x09) ? (b+'0') : (b+'A'-10));
}
bool HexToUint(char *S, uint8_t AMaxLength, uint32_t *AOutput) {
    *AOutput = 0;
    char c;
    uint8_t b=0;
    for(uint8_t i=0; i<AMaxLength; i++) {
        c = *S++;
        if (c == 0) return true;    // end of string
        // Shift result
        *AOutput <<= 4;
        // Get next digit
        if     ((c >= '0') && (c <= '9')) b = c-'0';
        else if((c >= 'A') && (c <= 'F')) b = c-'A'+10;
        else if((c >= 'a') && (c <= 'f')) b = c-'a'+10;
        else return false;  // not a hex digit
        *AOutput += b;
    }
    return true;
}

// ============================== High-level ===================================
// Send as digits
void UART_PrintUint (uint32_t ANumber) {
    uint8_t digit = '0';
    bool ShouldPrint = false;
    const uint32_t m[9] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10};

    for(uint8_t i=0; i<9; i++) {
        while (ANumber >= m[i]) {
            digit++;
            ANumber -= m[i];
        }
        if (digit != '0' || ShouldPrint) {
            UART_Print (digit);
            ShouldPrint = true;
        }
        digit = '0';
    } // for
    UART_Print ((uint8_t)('0'+ANumber));
}
void UART_PrintInt (int32_t ANumber) {
    if (ANumber < 0) {
        UART_Print ('-');
        ANumber = -ANumber;
    }
    else UART_Print (' ');
    UART_PrintUint (ANumber);
}

void UART_PrintString (const char *S) {
    while (*S != '\0') UART_Print (*S++);
}

void UART_Print8HexArray(uint8_t *Arr, uint32_t ALength) {
    for (uint32_t i=0; i<ALength; i++) {
        UART_PrintAsHex(Arr[i]);
        UART_Print(' ');
    }
}
