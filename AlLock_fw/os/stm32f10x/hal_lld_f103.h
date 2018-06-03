/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @defgroup STM32F103_HAL STM32F103 HAL Support
 * @details HAL support for STM32 Performance Line LD, MD and HD sub-families.
 *
 * @ingroup HAL
 */

/**
 * @file    STM32F1xx/hal_lld_f103.h
 * @brief   STM32F103 Performance Line HAL subsystem low level driver header.
 *
 * @addtogroup STM32F103_HAL
 * @{
 */

#ifndef _HAL_LLD_F103_H_
#define _HAL_LLD_F103_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Absolute Maximum Ratings
 * @{
 */
/**
 * @brief   Maximum system clock frequency.
 */
#define STM32_SYSCLK_MAX        72000000

/**
 * @brief   Maximum HSE clock frequency.
 */
#define STM32_HSECLK_MAX        25000000

/**
 * @brief   Minimum HSE clock frequency.
 */
#define STM32_HSECLK_MIN        1000000

/**
 * @brief   Maximum LSE clock frequency.
 */
#define STM32_LSECLK_MAX        1000000

/**
 * @brief   Minimum LSE clock frequency.
 */
#define STM32_LSECLK_MIN        32768

/**
 * @brief   Maximum PLLs input clock frequency.
 */
#define STM32_PLLIN_MAX         25000000

/**
 * @brief   Minimum PLLs input clock frequency.
 */
#define STM32_PLLIN_MIN         1000000

/**
 * @brief   Maximum PLL output clock frequency.
 */
#define STM32_PLLOUT_MAX        72000000

/**
 * @brief   Minimum PLL output clock frequency.
 */
#define STM32_PLLOUT_MIN        16000000

/**
 * @brief   Maximum APB1 clock frequency.
 */
#define STM32_PCLK1_MAX         36000000

/**
 * @brief   Maximum APB2 clock frequency.
 */
#define STM32_PCLK2_MAX         72000000

/**
 * @brief   Maximum ADC clock frequency.
 */
#define STM32_ADCCLK_MAX        14000000
/** @} */

/**
 * @name    RCC_CFGR register bits definitions
 * @{
 */
#define STM32_SW_HSI            (0 << 0)    /**< SYSCLK source is HSI.      */
#define STM32_SW_HSE            (1 << 0)    /**< SYSCLK source is HSE.      */
#define STM32_SW_PLL            (2 << 0)    /**< SYSCLK source is PLL.      */

#define STM32_HPRE_DIV1         (0 << 4)    /**< SYSCLK divided by 1.       */
#define STM32_HPRE_DIV2         (8 << 4)    /**< SYSCLK divided by 2.       */
#define STM32_HPRE_DIV4         (9 << 4)    /**< SYSCLK divided by 4.       */
#define STM32_HPRE_DIV8         (10 << 4)   /**< SYSCLK divided by 8.       */
#define STM32_HPRE_DIV16        (11 << 4)   /**< SYSCLK divided by 16.      */
#define STM32_HPRE_DIV64        (12 << 4)   /**< SYSCLK divided by 64.      */
#define STM32_HPRE_DIV128       (13 << 4)   /**< SYSCLK divided by 128.     */
#define STM32_HPRE_DIV256       (14 << 4)   /**< SYSCLK divided by 256.     */
#define STM32_HPRE_DIV512       (15 << 4)   /**< SYSCLK divided by 512.     */

#define STM32_PPRE1_DIV1        (0 << 8)    /**< HCLK divided by 1.         */
#define STM32_PPRE1_DIV2        (4 << 8)    /**< HCLK divided by 2.         */
#define STM32_PPRE1_DIV4        (5 << 8)    /**< HCLK divided by 4.         */
#define STM32_PPRE1_DIV8        (6 << 8)    /**< HCLK divided by 8.         */
#define STM32_PPRE1_DIV16       (7 << 8)    /**< HCLK divided by 16.        */

#define STM32_PPRE2_DIV1        (0 << 11)   /**< HCLK divided by 1.         */
#define STM32_PPRE2_DIV2        (4 << 11)   /**< HCLK divided by 2.         */
#define STM32_PPRE2_DIV4        (5 << 11)   /**< HCLK divided by 4.         */
#define STM32_PPRE2_DIV8        (6 << 11)   /**< HCLK divided by 8.         */
#define STM32_PPRE2_DIV16       (7 << 11)   /**< HCLK divided by 16.        */

#define STM32_ADCPRE_DIV2       (0 << 14)   /**< PPRE2 divided by 2.        */
#define STM32_ADCPRE_DIV4       (1 << 14)   /**< PPRE2 divided by 4.        */
#define STM32_ADCPRE_DIV6       (2 << 14)   /**< PPRE2 divided by 6.        */
#define STM32_ADCPRE_DIV8       (3 << 14)   /**< PPRE2 divided by 8.        */

#define STM32_PLLSRC_HSI        (0 << 16)   /**< PLL clock source is HSI.   */
#define STM32_PLLSRC_HSE        (1 << 16)   /**< PLL clock source is HSE.   */

#define STM32_PLLXTPRE_DIV1     (0 << 17)   /**< HSE divided by 1.          */
#define STM32_PLLXTPRE_DIV2     (1 << 17)   /**< HSE divided by 2.          */

#define STM32_USBPRE_DIV1P5     (0 << 22)   /**< PLLOUT divided by 1.5.     */
#define STM32_USBPRE_DIV1       (1 << 22)   /**< PLLOUT divided by 1.       */

#define STM32_MCOSEL_NOCLOCK    (0 << 24)   /**< No clock on MCO pin.       */
#define STM32_MCOSEL_SYSCLK     (4 << 24)   /**< SYSCLK on MCO pin.         */
#define STM32_MCOSEL_HSI        (5 << 24)   /**< HSI clock on MCO pin.      */
#define STM32_MCOSEL_HSE        (6 << 24)   /**< HSE clock on MCO pin.      */
#define STM32_MCOSEL_PLLDIV2    (7 << 24)   /**< PLL/2 clock on MCO pin.    */
/** @} */

/**
 * @name    RCC_BDCR register bits definitions
 * @{
 */
#define STM32_RTCSEL_MASK       (3 << 8)    /**< RTC clock source mask.     */
#define STM32_RTCSEL_NOCLOCK    (0 << 8)    /**< No clock.                  */
#define STM32_RTCSEL_LSE        (1 << 8)    /**< LSE used as RTC clock.     */
#define STM32_RTCSEL_LSI        (2 << 8)    /**< LSI used as RTC clock.     */
#define STM32_RTCSEL_HSEDIV     (3 << 8)    /**< HSE divided by 128 used as
                                                 RTC clock.                 */
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   Main clock source selection.
 * @note    If the selected clock source is not the PLL then the PLL is not
 *          initialized and started.
 * @note    The default value is calculated for a 72MHz system clock from
 *          a 8MHz crystal using the PLL.
 */
#if !defined(STM32_SW) || defined(__DOXYGEN__)
#define STM32_SW                    STM32_SW_PLL
#endif

/**
 * @brief   Clock source for the PLL.
 * @note    This setting has only effect if the PLL is selected as the
 *          system clock source.
 * @note    The default value is calculated for a 72MHz system clock from
 *          a 8MHz crystal using the PLL.
 */
#if !defined(STM32_PLLSRC) || defined(__DOXYGEN__)
#define STM32_PLLSRC                STM32_PLLSRC_HSE
#endif

/**
 * @brief   Crystal PLL pre-divider.
 * @note    This setting has only effect if the PLL is selected as the
 *          system clock source.
 * @note    The default value is calculated for a 72MHz system clock from
 *          a 8MHz crystal using the PLL.
 */
#if !defined(STM32_PLLXTPRE) || defined(__DOXYGEN__)
#define STM32_PLLXTPRE              STM32_PLLXTPRE_DIV1
#endif

/**
 * @brief   PLL multiplier value.
 * @note    The allowed range is 2...16.
 * @note    The default value is calculated for a 72MHz system clock from
 *          a 8MHz crystal using the PLL.
 */
#if !defined(STM32_PLLMUL_VALUE) || defined(__DOXYGEN__)
#define STM32_PLLMUL_VALUE          9
#endif

/**
 * @brief   AHB prescaler value.
 * @note    The default value is calculated for a 72MHz system clock from
 *          a 8MHz crystal using the PLL.
 */
#if !defined(STM32_HPRE) || defined(__DOXYGEN__)
#define STM32_HPRE                  STM32_HPRE_DIV1
#endif

/**
 * @brief   APB1 prescaler value.
 */
#if !defined(STM32_PPRE1) || defined(__DOXYGEN__)
#define STM32_PPRE1                 STM32_PPRE1_DIV2
#endif

/**
 * @brief   APB2 prescaler value.
 */
#if !defined(STM32_PPRE2) || defined(__DOXYGEN__)
#define STM32_PPRE2                 STM32_PPRE2_DIV2
#endif

/**
 * @brief   ADC prescaler value.
 */
#if !defined(STM32_ADCPRE) || defined(__DOXYGEN__)
#define STM32_ADCPRE                STM32_ADCPRE_DIV4
#endif

/**
 * @brief   USB clock setting.
 */
#if !defined(STM32_USB_CLOCK_REQUIRED) || defined(__DOXYGEN__)
#define STM32_USB_CLOCK_REQUIRED    TRUE
#endif

/**
 * @brief   USB prescaler initialization.
 */
#if !defined(STM32_USBPRE) || defined(__DOXYGEN__)
#define STM32_USBPRE                STM32_USBPRE_DIV1P5
#endif

/**
 * @brief   MCO pin setting.
 */
#if !defined(STM32_MCOSEL) || defined(__DOXYGEN__)
#define STM32_MCOSEL                STM32_MCOSEL_NOCLOCK
#endif

/**
 * @brief   RTC clock source.
 */
#if !defined(STM32_RTCSEL) || defined(__DOXYGEN__)
#define STM32_RTCSEL                STM32_RTCSEL_LSI
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*
 * Configuration-related checks.
 */
#if !defined(STM32F103_MCUCONF)
#error "Using a wrong mcuconf.h file, STM32F103_MCUCONF not defined"
#endif



#endif /* _HAL_LLD_F103_H_ */

/** @} */
