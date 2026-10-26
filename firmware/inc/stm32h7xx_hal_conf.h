#ifndef STM32H7XX_HAL_CONF_H
#define STM32H7XX_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_DFSDM_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_SAI_MODULE_ENABLED
#define HAL_SD_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED

#define HSE_VALUE               (25000000UL)
#define HSE_STARTUP_TIMEOUT     (100UL)
#define HSI_VALUE               (64000000UL)
#define CSI_VALUE               (4000000UL)
#define LSI_VALUE               (32000UL)
#define LSE_VALUE               (32768UL)
#define LSE_STARTUP_TIMEOUT     (5000UL)

#define EXTERNAL_CLOCK_VALUE    (12288000UL)

#define VDD_VALUE               (3300UL)
#define TICK_INT_PRIORITY       (0UL)
#define USE_RTOS                0
#define PREFETCH_ENABLE         1
#define INSTRUCTION_CACHE_ENABLE 1
#define DATA_CACHE_ENABLE       1

#define USE_SPI_CRC             0
#define USE_SD_TRANSCEIVER      0

#define USE_HAL_DMA_REGISTER_CALLBACKS     0
#define USE_HAL_DFSDM_REGISTER_CALLBACKS   0
#define USE_HAL_GPIO_REGISTER_CALLBACKS    0
#define USE_HAL_I2C_REGISTER_CALLBACKS     0
#define USE_HAL_PWR_REGISTER_CALLBACKS     0
#define USE_HAL_RCC_REGISTER_CALLBACKS     0
#define USE_HAL_SAI_REGISTER_CALLBACKS     0
#define USE_HAL_SD_REGISTER_CALLBACKS      0
#define USE_HAL_SPI_REGISTER_CALLBACKS     0
#define USE_HAL_UART_REGISTER_CALLBACKS    0

#ifdef HAL_RCC_MODULE_ENABLED
#include "stm32h7xx_hal_rcc.h"
#include "stm32h7xx_hal_rcc_ex.h"
#endif

#ifdef HAL_FLASH_MODULE_ENABLED
#include "stm32h7xx_hal_flash.h"
#endif

#ifdef HAL_GPIO_MODULE_ENABLED
#include "stm32h7xx_hal_gpio.h"
#endif

#ifdef HAL_DMA_MODULE_ENABLED
#include "stm32h7xx_hal_dma.h"
#include "stm32h7xx_hal_dma_ex.h"
#endif

#ifdef HAL_CORTEX_MODULE_ENABLED
#include "stm32h7xx_hal_cortex.h"
#endif

#ifdef HAL_PWR_MODULE_ENABLED
#include "stm32h7xx_hal_pwr.h"
#include "stm32h7xx_hal_pwr_ex.h"
#endif

#ifdef HAL_I2C_MODULE_ENABLED
#include "stm32h7xx_hal_i2c.h"
#include "stm32h7xx_hal_i2c_ex.h"
#endif

#ifdef HAL_SPI_MODULE_ENABLED
#include "stm32h7xx_hal_spi.h"
#endif

#ifdef HAL_UART_MODULE_ENABLED
#include "stm32h7xx_hal_uart.h"
#endif

#ifdef HAL_SAI_MODULE_ENABLED
#include "stm32h7xx_hal_sai.h"
#include "stm32h7xx_hal_sai_ex.h"
#endif

#ifdef HAL_SD_MODULE_ENABLED
#include "stm32h7xx_hal_sd.h"
#include "stm32h7xx_hal_sd_ex.h"
#endif

#ifdef HAL_DFSDM_MODULE_ENABLED
#include "stm32h7xx_hal_dfsdm.h"
#include "stm32h7xx_hal_dfsdm_ex.h"
#endif

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line);
#define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
#else
#define assert_param(expr) ((void)0U)
#endif

#ifdef __cplusplus
}
#endif

#endif
