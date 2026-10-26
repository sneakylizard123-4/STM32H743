#include "ulpi.h"

#include <stddef.h>

#include "board.h"
#include "console.h"
#include "stm32h7xx_hal.h"

#include <stddef.h>

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} ulpi_pin_t;

static const ulpi_pin_t ulpi_data[8] = {
    {GPIOA, GPIO_PIN_3},
    {GPIOB, GPIO_PIN_0},
    {GPIOB, GPIO_PIN_1},
    {GPIOB, GPIO_PIN_10},
    {GPIOB, GPIO_PIN_11},
    {GPIOB, GPIO_PIN_12},
    {GPIOB, GPIO_PIN_13},
    {GPIOB, GPIO_PIN_5}
};

#define ULPI_NXT_PORT GPIOH
#define ULPI_NXT_PIN GPIO_PIN_4
#define ULPI_STP_PORT GPIOC
#define ULPI_STP_PIN GPIO_PIN_0
#define ULPI_DIR_PORT GPIOI
#define ULPI_DIR_PIN GPIO_PIN_11
#define ULPI_RESET_PORT GPIOH
#define ULPI_RESET_PIN GPIO_PIN_12

static void ulpi_data_output(void)
{
    GPIO_InitTypeDef gpio = {0};
    uint32_t i;

    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    for (i = 0U; i < 8U; i++)
    {
        gpio.Pin = ulpi_data[i].pin;
        HAL_GPIO_Init(ulpi_data[i].port, &gpio);
    }
}

static void ulpi_data_input(void)
{
    GPIO_InitTypeDef gpio = {0};
    uint32_t i;

    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLDOWN;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    for (i = 0U; i < 8U; i++)
    {
        gpio.Pin = ulpi_data[i].pin;
        HAL_GPIO_Init(ulpi_data[i].port, &gpio);
    }
}

static void ulpi_write_data(uint8_t val)
{
    uint32_t i;

    for (i = 0U; i < 8U; i++)
    {
        HAL_GPIO_WritePin(ulpi_data[i].port,
                          ulpi_data[i].pin,
                          (val & (1U << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

static uint8_t ulpi_read_data(void)
{
    uint8_t val = 0U;
    uint32_t i;

    for (i = 0U; i < 8U; i++)
    {
        if (HAL_GPIO_ReadPin(ulpi_data[i].port, ulpi_data[i].pin) == GPIO_PIN_SET)
        {
            val |= (uint8_t)(1U << i);
        }
    }

    return val;
}

static int ulpi_wait_dir(int level, uint32_t timeout_us)
{
    uint32_t n;

    for (n = 0U; n < timeout_us; n++)
    {
        if ((HAL_GPIO_ReadPin(ULPI_DIR_PORT, ULPI_DIR_PIN) == GPIO_PIN_SET) ==
            (level != 0))
        {
            return 0;
        }
        board_delay_us(1U);
    }

    return -1;
}

static int ulpi_wait_nxt(void)
{
    uint32_t n;

    for (n = 0U; n < 1000U; n++)
    {
        if (HAL_GPIO_ReadPin(ULPI_NXT_PORT, ULPI_NXT_PIN) == GPIO_PIN_SET)
        {
            return 0;
        }
        board_delay_us(1U);
    }

    return -1;
}

static void ulpi_idle(void)
{
    HAL_GPIO_WritePin(ULPI_STP_PORT, ULPI_STP_PIN, GPIO_PIN_RESET);
    ulpi_data_output();
    ulpi_write_data(0x00U);
}

void ulpi_reset(void)
{
    HAL_GPIO_WritePin(ULPI_RESET_PORT, ULPI_RESET_PIN, GPIO_PIN_RESET);
    HAL_Delay(10U);
    HAL_GPIO_WritePin(ULPI_RESET_PORT, ULPI_RESET_PIN, GPIO_PIN_SET);
    HAL_Delay(10U);
}

int ulpi_read_reg(uint8_t reg, uint8_t *val)
{
    if (val == NULL)
    {
        return -1;
    }

    if (ulpi_wait_dir(0, 1000U) != 0)
    {
        return -1;
    }

    ulpi_data_output();
    ulpi_write_data((uint8_t)(0xC0U | (reg & 0x3FU)));
    HAL_GPIO_WritePin(ULPI_STP_PORT, ULPI_STP_PIN, GPIO_PIN_RESET);

    if (ulpi_wait_nxt() != 0)
    {
        ulpi_idle();
        return -1;
    }

    ulpi_data_input();

    if (ulpi_wait_dir(1, 1000U) != 0)
    {
        ulpi_idle();
        return -1;
    }

    *val = ulpi_read_data();

    if (ulpi_wait_dir(0, 1000U) != 0)
    {
        ulpi_idle();
        return -1;
    }

    ulpi_idle();
    return 0;
}

int ulpi_write_reg(uint8_t reg, uint8_t val)
{
    if (ulpi_wait_dir(0, 1000U) != 0)
    {
        return -1;
    }

    ulpi_data_output();
    ulpi_write_data((uint8_t)(0x80U | (reg & 0x3FU)));
    HAL_GPIO_WritePin(ULPI_STP_PORT, ULPI_STP_PIN, GPIO_PIN_RESET);

    if (ulpi_wait_nxt() != 0)
    {
        ulpi_idle();
        return -1;
    }

    ulpi_write_data(val);

    if (ulpi_wait_nxt() != 0)
    {
        ulpi_idle();
        return -1;
    }

    HAL_GPIO_WritePin(ULPI_STP_PORT, ULPI_STP_PIN, GPIO_PIN_SET);
    board_delay_us(1U);
    ulpi_idle();

    return 0;
}

int ulpi_test(void)
{
    GPIO_InitTypeDef gpio = {0};
    uint8_t regs[4];
    uint32_t i;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLDOWN;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Pin = ULPI_NXT_PIN;
    HAL_GPIO_Init(ULPI_NXT_PORT, &gpio);

    gpio.Pin = ULPI_DIR_PIN;
    HAL_GPIO_Init(ULPI_DIR_PORT, &gpio);

    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Pin = ULPI_STP_PIN;
    HAL_GPIO_Init(ULPI_STP_PORT, &gpio);
    HAL_GPIO_WritePin(ULPI_STP_PORT, ULPI_STP_PIN, GPIO_PIN_RESET);

    gpio.Pin = ULPI_RESET_PIN;
    HAL_GPIO_Init(ULPI_RESET_PORT, &gpio);
    HAL_GPIO_WritePin(ULPI_RESET_PORT, ULPI_RESET_PIN, GPIO_PIN_SET);

    ulpi_idle();
    ulpi_reset();

    for (i = 0U; i < 4U; i++)
    {
        if (ulpi_read_reg((uint8_t)i, &regs[i]) != 0)
        {
            console_printf("ULPI: register %lu read failed\n",
                           (unsigned long)i);
            return -1;
        }
    }

    if ((regs[0] == 0x00U) && (regs[1] == 0x00U) &&
        (regs[2] == 0x00U) && (regs[3] == 0x00U))
    {
        console_puts("ULPI: bus reads all zero (no PHY or no 60 MHz clock)\n");
        return -1;
    }

    console_printf("ULPI: vendor %02X%02X product %02X%02X\n",
                   regs[1], regs[0], regs[3], regs[2]);
    console_puts("ULPI: register probe only, no USB stack\n");
    return 0;
}
