#include "clock.h"

#include "stm32h7xx_hal.h"

int clock_init(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};
    RCC_PeriphCLKInitTypeDef periph = {0};
    uint32_t start;

    if (HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY) != HAL_OK)
    {
        return -1;
    }

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    start = HAL_GetTick();
    while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY) == 0U)
    {
        if ((HAL_GetTick() - start) > 100U)
        {
            return -1;
        }
    }

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLM = 5U;
    osc.PLL.PLLN = 160U;
    osc.PLL.PLLP = 2U;
    osc.PLL.PLLQ = 8U;
    osc.PLL.PLLR = 2U;
    osc.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    osc.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    osc.PLL.PLLFRACN = 0U;

    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        return -1;
    }

    clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                    RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 |
                    RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.SYSCLKDivider = RCC_SYSCLK_DIV1;
    clk.AHBCLKDivider = RCC_HCLK_DIV2;
    clk.APB3CLKDivider = RCC_APB3_DIV2;
    clk.APB1CLKDivider = RCC_APB1_DIV2;
    clk.APB2CLKDivider = RCC_APB2_DIV2;
    clk.APB4CLKDivider = RCC_APB4_DIV2;

    if (HAL_RCC_ClockConfig(&clk, FLASH_ACR_LATENCY_4WS) != HAL_OK)
    {
        return -1;
    }

    periph.PeriphClockSelection = RCC_PERIPHCLK_SAI2 |
                                  RCC_PERIPHCLK_SDMMC |
                                  RCC_PERIPHCLK_SPI1 |
                                  RCC_PERIPHCLK_I2C1;

    periph.PLL2.PLL2M = 5U;
    periph.PLL2.PLL2N = 49U;
    periph.PLL2.PLL2P = 20U;
    periph.PLL2.PLL2Q = 2U;
    periph.PLL2.PLL2R = 2U;
    periph.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
    periph.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    periph.PLL2.PLL2FRACN = 1245U;

    periph.Sai23ClockSelection = RCC_SAI23CLKSOURCE_PLL2;
    periph.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
    periph.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
    periph.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;

    if (HAL_RCCEx_PeriphCLKConfig(&periph) != HAL_OK)
    {
        return -1;
    }

    SystemCoreClockUpdate();

    if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK)
    {
        return -1;
    }

    return 0;
}
