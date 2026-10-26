#include "fpga.h"

#include "board.h"
#include "stm32h7xx_hal.h"

#define FPGA_NSS_PORT GPIOA
#define FPGA_NSS_PIN GPIO_PIN_15
#define FPGA_CRESET_PORT GPIOD
#define FPGA_CRESET_PIN GPIO_PIN_15
#define FPGA_CDONE_PORT GPIOD
#define FPGA_CDONE_PIN GPIO_PIN_14

extern const uint8_t fpga_bitstream[];
extern const uint32_t fpga_bitstream_len;

static SPI_HandleTypeDef hspi1;

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef gpio = {0};

    if (hspi->Instance != SPI1)
    {
        return;
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();

    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    gpio.Pin = GPIO_PIN_3 | GPIO_PIN_4;
    gpio.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &gpio);

    gpio.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOD, &gpio);

    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Pin = FPGA_NSS_PIN;
    HAL_GPIO_Init(FPGA_NSS_PORT, &gpio);
    HAL_GPIO_WritePin(FPGA_NSS_PORT, FPGA_NSS_PIN, GPIO_PIN_SET);

    gpio.Pin = FPGA_CRESET_PIN;
    HAL_GPIO_Init(FPGA_CRESET_PORT, &gpio);
    HAL_GPIO_WritePin(FPGA_CRESET_PORT, FPGA_CRESET_PIN, GPIO_PIN_SET);

    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLDOWN;
    gpio.Pin = FPGA_CDONE_PIN;
    HAL_GPIO_Init(FPGA_CDONE_PORT, &gpio);
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance != SPI1)
    {
        return;
    }

    __HAL_RCC_SPI1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, FPGA_NSS_PIN);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3 | GPIO_PIN_4);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_7 | FPGA_CDONE_PIN | FPGA_CRESET_PIN);
}

int fpga_cdone(void)
{
    return HAL_GPIO_ReadPin(FPGA_CDONE_PORT, FPGA_CDONE_PIN) == GPIO_PIN_SET;
}

void fpga_reset(void)
{
    HAL_GPIO_WritePin(FPGA_CRESET_PORT, FPGA_CRESET_PIN, GPIO_PIN_RESET);
    board_delay_us(2000U);
    HAL_GPIO_WritePin(FPGA_CRESET_PORT, FPGA_CRESET_PIN, GPIO_PIN_SET);
    board_delay_us(2000U);
}

int fpga_load(void)
{
    uint32_t offset = 0U;
    uint32_t start;
    uint8_t dummy[8] = {0xFFU, 0xFFU, 0xFFU, 0xFFU,
                        0xFFU, 0xFFU, 0xFFU, 0xFFU};

    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 7U;
    hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;

    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        return -1;
    }

    fpga_reset();

    if (fpga_cdone())
    {
        return -1;
    }

    HAL_GPIO_WritePin(FPGA_NSS_PORT, FPGA_NSS_PIN, GPIO_PIN_RESET);
    board_delay_us(100U);

    while (offset < fpga_bitstream_len)
    {
        uint16_t chunk = (uint16_t)((fpga_bitstream_len - offset) > 4096U
                                        ? 4096U
                                        : (fpga_bitstream_len - offset));

        if (HAL_SPI_Transmit(&hspi1,
                             (uint8_t *)&fpga_bitstream[offset],
                             chunk,
                             1000U) != HAL_OK)
        {
            HAL_GPIO_WritePin(FPGA_NSS_PORT, FPGA_NSS_PIN, GPIO_PIN_SET);
            return -1;
        }

        offset += chunk;
    }

    (void)HAL_SPI_Transmit(&hspi1, dummy, sizeof(dummy), 100U);
    HAL_GPIO_WritePin(FPGA_NSS_PORT, FPGA_NSS_PIN, GPIO_PIN_SET);

    start = HAL_GetTick();
    while (!fpga_cdone())
    {
        if ((HAL_GetTick() - start) > 100U)
        {
            return -1;
        }
    }

    return 0;
}
