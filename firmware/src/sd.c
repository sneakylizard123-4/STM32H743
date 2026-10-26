#include "sd.h"

#include "console.h"
#include "stm32h7xx_hal.h"

SD_HandleTypeDef hsd1;

void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
    GPIO_InitTypeDef gpio = {0};

    if (hsd->Instance != SDMMC1)
    {
        return;
    }

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_SDMMC1_CLK_ENABLE();

    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF12_SDMMC1;

    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
               GPIO_PIN_11 | GPIO_PIN_12;
    HAL_GPIO_Init(GPIOC, &gpio);

    gpio.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOD, &gpio);

    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    gpio.Pin = GPIO_PIN_4;
    HAL_GPIO_Init(GPIOD, &gpio);
}

void HAL_SD_MspDeInit(SD_HandleTypeDef *hsd)
{
    if (hsd->Instance != SDMMC1)
    {
        return;
    }

    __HAL_RCC_SDMMC1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                              GPIO_PIN_11 | GPIO_PIN_12);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2 | GPIO_PIN_4);
}

int sd_test(void)
{
    HAL_SD_CardCIDTypeDef cid;
    HAL_SD_CardCSDTypeDef csd;
    uint64_t capacity;
    HAL_StatusTypeDef status;

    hsd1.Instance = SDMMC1;
    hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;
    hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd1.Init.ClockDiv = 0U;

    status = HAL_SD_Init(&hsd1);
    if (status != HAL_OK)
    {
        console_printf("SD: no card or init failed (%d)\n", (int)status);
        return -1;
    }

    if ((HAL_SD_GetCardCID(&hsd1, &cid) != HAL_OK) ||
        (HAL_SD_GetCardCSD(&hsd1, &csd) != HAL_OK))
    {
        console_puts("SD: CID/CSD read failed\n");
        return -1;
    }

    capacity = (uint64_t)hsd1.SdCard.LogBlockNbr *
               (uint64_t)hsd1.SdCard.LogBlockSize;

    console_printf("SD: CID mfr=%02X oem=%04X name=%08lX rev=%u "
                   "serial=%08lX date=%04X\n",
                   cid.ManufacturerID,
                   cid.OEM_AppliID,
                   (unsigned long)cid.ProdName1,
                   cid.ProdRev,
                   (unsigned long)cid.ProdSN,
                   cid.ManufactDate);
    console_printf("SD: CSD version=%u block=%lu bytes, capacity=%lu MiB\n",
                   csd.CSDStruct,
                   (unsigned long)hsd1.SdCard.LogBlockSize,
                   (unsigned long)(capacity / (1024ULL * 1024ULL)));

    status = HAL_SD_ConfigWideBusOperation(&hsd1, SDMMC_BUS_WIDE_4B);
    if (status == HAL_OK)
    {
        console_puts("SD: 4-bit bus enabled\n");
    }
    else
    {
        console_puts("SD: 4-bit bus unavailable, staying 1-bit\n");
    }

    return 0;
}
