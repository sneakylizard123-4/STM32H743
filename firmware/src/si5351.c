#include "si5351.h"

#include "board.h"

#define SI5351_I2C_ADDR (0x60U << 1)
#define SI5351_REG_OUTPUT_ENABLE 3U
#define SI5351_REG_OEB_ENABLE 9U
#define SI5351_REG_CLK0_CONTROL 16U
#define SI5351_REG_PLLA 26U
#define SI5351_REG_PLLB 34U
#define SI5351_REG_MS0 42U
#define SI5351_REG_MS6 90U
#define SI5351_REG_MS7 91U
#define SI5351_REG_PLL_RESET 177U
#define SI5351_REG_XTAL_LOAD 183U

I2C_HandleTypeDef hi2c1;

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef gpio = {0};

    if (hi2c->Instance != I2C1)
    {
        return;
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gpio);
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance != I2C1)
    {
        return;
    }

    __HAL_RCC_I2C1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);
}

static int si5351_write(uint8_t reg, uint8_t val)
{
    return (HAL_I2C_Mem_Write(&hi2c1,
                              SI5351_I2C_ADDR,
                              reg,
                              I2C_MEMADD_SIZE_8BIT,
                              &val,
                              1,
                              100U) == HAL_OK) ? 0 : -1;
}

static int si5351_write_n(uint8_t reg, const uint8_t *buf, uint16_t len)
{
    return (HAL_I2C_Mem_Write(&hi2c1,
                              SI5351_I2C_ADDR,
                              reg,
                              I2C_MEMADD_SIZE_8BIT,
                              (uint8_t *)buf,
                              len,
                              100U) == HAL_OK) ? 0 : -1;
}

int si5351_read_reg(uint8_t reg, uint8_t *val)
{
    return (HAL_I2C_Mem_Read(&hi2c1,
                             SI5351_I2C_ADDR,
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             val,
                             1,
                             100U) == HAL_OK) ? 0 : -1;
}

int si5351_write_reg(uint8_t reg, uint8_t val)
{
    return si5351_write(reg, val);
}

static int si5351_pll_config(uint8_t base, uint32_t mult,
                             uint32_t num, uint32_t den)
{
    uint32_t p1;
    uint32_t p2;
    uint32_t p3;
    uint8_t b[8];

    p1 = (128U * mult) + ((128U * num) / den) - 512U;
    p2 = (128U * num) - (den * ((128U * num) / den));
    p3 = den;

    b[0] = (uint8_t)(p3 >> 8);
    b[1] = (uint8_t)p3;
    b[2] = (uint8_t)((p1 >> 16) & 0x03U);
    b[3] = (uint8_t)(p1 >> 8);
    b[4] = (uint8_t)p1;
    b[5] = (uint8_t)(((p3 & 0xF0000U) >> 12) | ((p2 & 0xF0000U) >> 16));
    b[6] = (uint8_t)(p2 >> 8);
    b[7] = (uint8_t)p2;

    return si5351_write_n(base, b, sizeof(b));
}

static int si5351_ms_config(uint8_t output, uint32_t div,
                            uint32_t num, uint32_t den)
{
    uint32_t p1;
    uint32_t p2;
    uint32_t p3;
    uint8_t base;
    uint8_t b[8];

    p1 = (128U * div) + ((128U * num) / den) - 512U;
    p2 = (128U * num) - (den * ((128U * num) / den));
    p3 = den;

    base = (uint8_t)(SI5351_REG_MS0 + (output * 8U));

    b[0] = (uint8_t)(p3 >> 8);
    b[1] = (uint8_t)p3;
    b[2] = (uint8_t)((p1 >> 16) & 0x03U);
    b[3] = (uint8_t)(p1 >> 8);
    b[4] = (uint8_t)p1;
    b[5] = (uint8_t)(((p3 & 0xF0000U) >> 12) | ((p2 & 0xF0000U) >> 16));
    b[6] = (uint8_t)(p2 >> 8);
    b[7] = (uint8_t)p2;

    return si5351_write_n(base, b, sizeof(b));
}

static int si5351_output_config(uint8_t output, int pll_b,
                                uint32_t div, int integer)
{
    uint8_t ctrl;

    if (output < 6U)
    {
        if (si5351_ms_config(output, div, 0U, 1U) != 0)
        {
            return -1;
        }

        ctrl = 0x0FU;
        if (pll_b)
        {
            ctrl |= 0x20U;
        }
        if (integer)
        {
            ctrl |= 0x40U;
        }
    }
    else
    {
        if ((div < 6U) || (div > 254U) || ((div & 1U) != 0U))
        {
            return -1;
        }

        if (si5351_write((uint8_t)(SI5351_REG_MS6 + (output - 6U)),
                         (uint8_t)div) != 0)
        {
            return -1;
        }

        ctrl = 0x0CU | 0x40U;
        if (pll_b)
        {
            ctrl |= 0x20U;
        }
    }

    return si5351_write((uint8_t)(SI5351_REG_CLK0_CONTROL + output), ctrl);
}

int si5351_locked(void)
{
    uint8_t status;

    if (si5351_read_reg(0U, &status) != 0)
    {
        return 0;
    }

    return (status & 0x60U) == 0U;
}

int si5351_init(void)
{
    uint32_t i;

    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10707DBCU;
    hi2c1.Init.OwnAddress1 = 0U;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0U;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        return -1;
    }

    if (si5351_write(SI5351_REG_OUTPUT_ENABLE, 0xFFU) != 0)
    {
        return -1;
    }

    /* OEB is pulled high on rev A. Reg 9 = 0xFF makes the outputs ignore it. */
    if (si5351_write(SI5351_REG_OEB_ENABLE, 0xFFU) != 0)
    {
        return -1;
    }

    if (si5351_write(15U, 0x00U) != 0)
    {
        return -1;
    }

    if (si5351_write(SI5351_REG_XTAL_LOAD, 0x00U) != 0)
    {
        return -1;
    }

    if (si5351_pll_config(SI5351_REG_PLLA, 36U, 0U, 1U) != 0)
    {
        return -1;
    }

    if (si5351_pll_config(SI5351_REG_PLLB, 30U, 18U, 25U) != 0)
    {
        return -1;
    }

    if (si5351_output_config(0U, 0, 36U, 1) != 0)
    {
        return -1;
    }
    if (si5351_output_config(1U, 0, 15U, 0) != 0)
    {
        return -1;
    }
    if (si5351_output_config(2U, 0, 75U, 0) != 0)
    {
        return -1;
    }
    if (si5351_output_config(3U, 0, 36U, 1) != 0)
    {
        return -1;
    }
    if (si5351_output_config(4U, 1, 32U, 1) != 0)
    {
        return -1;
    }
    if (si5351_output_config(5U, 1, 64U, 1) != 0)
    {
        return -1;
    }
    if (si5351_output_config(6U, 1, 16U, 1) != 0)
    {
        return -1;
    }
    if (si5351_output_config(7U, 1, 128U, 1) != 0)
    {
        return -1;
    }

    if (si5351_write(SI5351_REG_PLL_RESET, 0xACU) != 0)
    {
        return -1;
    }

    HAL_Delay(10U);

    for (i = 0U; i < 100U; i++)
    {
        if (si5351_locked())
        {
            break;
        }
        HAL_Delay(1U);
    }

    if (!si5351_locked())
    {
        return -1;
    }

    return (si5351_write(SI5351_REG_OUTPUT_ENABLE, 0x00U) == 0) ? 0 : -1;
}
