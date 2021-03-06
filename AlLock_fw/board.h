/*
 * board.h
 *
 *  Created on: 12 ����. 2015 �.
 *      Author: Kreyl
 */

#pragma once

#include <inttypes.h>

// ==== General ====
#define BOARD_NAME          "KeyLock01"
#define APP_NAME            "KeyLock"

// MCU type as defined in the ST header.
#define STM32F103xE

// Freq of external crystal if any. Leave it here even if not used.
#define CRYSTAL_FREQ_HZ         12000000

// OS timer settings
#define STM32_ST_IRQ_PRIORITY   2
#define STM32_ST_USE_TIMER      5
#define SYS_TIM_CLK             (Clk.APB1FreqHz)    // Timer 5 is clocked by APB1

//  Periphery
#define I2C1_ENABLED            TRUE

#define ADC_REQUIRED            TRUE
#define STM32_DMA_REQUIRED      TRUE    // Leave this macro name for OS

#if 1 // ========================== GPIO =======================================
// EXTI
#define INDIVIDUAL_EXTI_IRQ_REQUIRED    FALSE

#define BAT_MEAS_PIN    GPIOC, 0

// Coils
#define COIL_A_SETUP    {GPIOB, 8, TIM4, 3, invNotInverted, omPushPull, 7}
#define COIL_B_SETUP    {GPIOB, 9, TIM4, 4, invNotInverted, omPushPull, 7}

// Buttons
#define BTNS_GPIO       GPIOB
#define BTN_A1_PIN      1
#define BTN_A2_PIN      2
#define BTN_A3_PIN      10
#define BTN_B1_PIN      3
#define BTN_B2_PIN      4
#define BTN_B3_PIN      5
#define BTN_4_PIN       11
#define BTN_5_PIN       12
#define BTN_6_PIN       13
#define BTN_7_PIN       14

// UART
#define UART_GPIO       GPIOA
#define UART_TX_PIN     9
#define UART_RX_PIN     10

#define EXTUART_GPIO    GPIOA
#define EXTUART_TX_PIN  2
#define EXTUART_RX_PIN  3

// LED controller
#define LED_OE_PIN      GPIOA, 12

// Lasers
#define LASER_ON_PIN    GPIOA, 11

// VS
#define VS_XCS          GPIOA, 0
#define VS_XDCS         GPIOA, 4
#define VS_RST          GPIOA, 15
#define VS_DREQ         GPIOA, 1
#define VS_XCLK         GPIOA, 5
#define VS_SO           GPIOA, 6
#define VS_SI           GPIOA, 7

// I2C
#define I2C1_GPIO       GPIOB
#define I2C1_SCL        6
#define I2C1_SDA        7
#define I2C2_GPIO       GPIOB
#define I2C2_SCL        10
#define I2C2_SDA        11
#define I2C3_GPIO       GPIOC
#define I2C3_SCL        0
#define I2C3_SDA        1
// I2C Alternate Function
#define I2C_AF          AF4

// SD
#define SD_PWR_PIN      GPIOC, 7
#define SD_AF           AF1
#define SD_DAT0         GPIOC,  8, omPushPull, pudPullUp, SD_AF
#define SD_DAT1         GPIOC,  9, omPushPull, pudPullUp, SD_AF
#define SD_DAT2         GPIOC, 10, omPushPull, pudPullUp, SD_AF
#define SD_DAT3         GPIOC, 11, omPushPull, pudPullUp, SD_AF
#define SD_CLK          GPIOC, 12, omPushPull, pudNone,   SD_AF
#define SD_CMD          GPIOD,  2, omPushPull, pudPullUp, SD_AF

#endif // GPIO

#if 1 // =========================== SPI =======================================
#define VS_SPI          SPI1
#endif

#if 1 // =========================== I2C =======================================
#define I2C1_BAUDRATE 400000
#endif

#if ADC_REQUIRED // ======================= Inner ADC ==========================
#define ADC_MEAS_PERIOD_MS  999
// Clock divider: clock is generated from the APB2
#define ADC_CLK_DIVIDER		adcDiv4

// ADC channels
#define ADC_BATTERY_CHNL 	10
#define ADC_VREFINT_CHNL    17

#define ADC_CHANNELS        { ADC_BATTERY_CHNL, ADC_VREFINT_CHNL }
#define ADC_CHANNEL_CNT     2   // Do not use countof(AdcChannels) as preprocessor does not know what is countof => cannot check
#define ADC_SAMPLE_TIME     ast55d5Cycles
#define ADC_SAMPLE_CNT      8   // How many times to measure every channel
#define ADC_SEQ_LEN         (ADC_SAMPLE_CNT * ADC_CHANNEL_CNT)
#endif

#if 1 // =========================== DMA =======================================
// ==== Uart ====
// Remap is made automatically if required
#define UART_DMA_TX     STM32_DMA1_STREAM4
#define UART_DMA_RX     STM32_DMA1_STREAM5
#define UART_DMA_CHNL   2 // dummy
#define UART_DMA_TX_MODE(Chnl) \
                            (STM32_DMA_CR_CHSEL(Chnl) | \
                            DMA_PRIORITY_LOW | \
                            STM32_DMA_CR_MSIZE_BYTE | \
                            STM32_DMA_CR_PSIZE_BYTE | \
                            STM32_DMA_CR_MINC |       /* Memory pointer increase */ \
                            STM32_DMA_CR_DIR_M2P |    /* Direction is memory to peripheral */ \
                            STM32_DMA_CR_TCIE         /* Enable Transmission Complete IRQ */)

#define UART_DMA_RX_MODE(Chnl) \
                            (STM32_DMA_CR_CHSEL((Chnl)) | \
                            DMA_PRIORITY_MEDIUM | \
                            STM32_DMA_CR_MSIZE_BYTE | \
                            STM32_DMA_CR_PSIZE_BYTE | \
                            STM32_DMA_CR_MINC |       /* Memory pointer increase */ \
                            STM32_DMA_CR_DIR_P2M |    /* Direction is peripheral to memory */ \
                            STM32_DMA_CR_CIRC         /* Circular buffer enable */)


#define EXTUART_DMA_TX  STM32_DMA1_STREAM7
#define EXTUART_DMA_RX  STM32_DMA1_STREAM6

#define VS_DMA          STM32_DMA1_STREAM3
#define VS_DMA_CHNL     0
#define VS_DMA_MODE     STM32_DMA_CR_CHSEL(VS_DMA_CHNL) | \
                        DMA_PRIORITY_LOW | \
                        STM32_DMA_CR_MSIZE_BYTE | \
                        STM32_DMA_CR_PSIZE_BYTE | \
                        STM32_DMA_CR_DIR_M2P |    /* Direction is memory to peripheral */ \
                        STM32_DMA_CR_TCIE         /* Enable Transmission Complete IRQ */


// ==== I2C ====
#define I2C1_DMA_TX     nullptr //STM32_DMA2_STREAM7
#define I2C1_DMA_RX     nullptr //STM32_DMA2_STREAM6
#define I2C1_DMA_CHNL   5
#define I2C2_DMA_TX     STM32_DMA1_STREAM4
#define I2C2_DMA_RX     STM32_DMA1_STREAM5
#define I2C2_DMA_CHNL   3
#define I2C3_DMA_TX     STM32_DMA1_STREAM2
#define I2C3_DMA_RX     STM32_DMA1_STREAM3
#define I2C3_DMA_CHNL   3

// ==== SDMMC ====
#define STM32_SDC_SDMMC1_DMA_STREAM   STM32_DMA_STREAM_ID(2, 5)

#if ADC_REQUIRED
#define ADC_DMA         STM32_DMA1_STREAM1
#define ADC_DMA_MODE    STM32_DMA_CR_CHSEL(0) |   /* DMA1 Stream1 Channel0 */ \
                        DMA_PRIORITY_LOW | \
                        STM32_DMA_CR_MSIZE_HWORD | \
                        STM32_DMA_CR_PSIZE_HWORD | \
                        STM32_DMA_CR_MINC |       /* Memory pointer increase */ \
                        STM32_DMA_CR_DIR_P2M |    /* Direction is peripheral to memory */ \
                        STM32_DMA_CR_TCIE         /* Enable Transmission Complete IRQ */
#endif // ADC

#endif // DMA

#if 1 // ========================== USART ======================================
#define PRINTF_FLOAT_EN FALSE
#define UART_TXBUF_SZ   1024
#define UART_RXBUF_SZ   99
#define CMD_UART_PARAMS \
    USART1, UART_GPIO, UART_TX_PIN, UART_GPIO, UART_RX_PIN, \
    UART_DMA_TX, UART_DMA_RX, UART_DMA_TX_MODE(UART_DMA_CHNL), UART_DMA_RX_MODE(UART_DMA_CHNL)

#define EXT_UART        USART2
#define EXT_UART_PARAMS \
    USART2, EXTUART_GPIO, EXTUART_TX_PIN, EXTUART_GPIO, EXTUART_RX_PIN, \
    EXTUART_DMA_TX, EXTUART_DMA_RX, UART_DMA_TX_MODE(UART_DMA_CHNL), UART_DMA_RX_MODE(UART_DMA_CHNL)

#endif

