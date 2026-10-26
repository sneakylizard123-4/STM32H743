#include "eth_phy.h"

#include <stddef.h>

#include "board.h"
#include "console.h"
#include "stm32h7xx_hal.h"

#include <stddef.h>

#define ETH_MDC_PORT GPIOC
#define ETH_MDC_PIN GPIO_PIN_1
#define ETH_MDIO_PORT GPIOA
#define ETH_MDIO_PIN GPIO_PIN_2
#define ETH_RST_PORT GPIOE
#define ETH_RST_PIN GPIO_PIN_3

static void mdio_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Pin = ETH_MDC_PIN;
    HAL_GPIO_Init(ETH_MDC_PORT, &gpio);
    HAL_GPIO_WritePin(ETH_MDC_PORT, ETH_MDC_PIN, GPIO_PIN_RESET);

    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Pin = ETH_MDIO_PIN;
    HAL_GPIO_Init(ETH_MDIO_PORT, &gpio);

    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Pin = ETH_RST_PIN;
    HAL_GPIO_Init(ETH_RST_PORT, &gpio);
    HAL_GPIO_WritePin(ETH_RST_PORT, ETH_RST_PIN, GPIO_PIN_SET);
}

static void mdio_write_bit(int bit)
{
    HAL_GPIO_WritePin(ETH_MDIO_PORT,
                      ETH_MDIO_PIN,
                      bit ? GPIO_PIN_SET : GPIO_PIN_RESET);
    board_delay_us(1U);
    HAL_GPIO_WritePin(ETH_MDC_PORT, ETH_MDC_PIN, GPIO_PIN_SET);
    board_delay_us(1U);
    HAL_GPIO_WritePin(ETH_MDC_PORT, ETH_MDC_PIN, GPIO_PIN_RESET);
    board_delay_us(1U);
}

static int mdio_read_bit(void)
{
    int bit;

    HAL_GPIO_WritePin(ETH_MDIO_PORT, ETH_MDIO_PIN, GPIO_PIN_SET);
    board_delay_us(1U);
    HAL_GPIO_WritePin(ETH_MDC_PORT, ETH_MDC_PIN, GPIO_PIN_SET);
    board_delay_us(1U);
    bit = HAL_GPIO_ReadPin(ETH_MDIO_PORT, ETH_MDIO_PIN) == GPIO_PIN_SET;
    HAL_GPIO_WritePin(ETH_MDC_PORT, ETH_MDC_PIN, GPIO_PIN_RESET);
    board_delay_us(1U);
    return bit;
}

int eth_phy_read(uint8_t phy, uint8_t reg, uint16_t *val)
{
    uint32_t i;
    uint16_t data = 0U;

    if (val == NULL)
    {
        return -1;
    }

    for (i = 0U; i < 32U; i++)
    {
        mdio_write_bit(1);
    }

    mdio_write_bit(0);
    mdio_write_bit(1);

    mdio_write_bit(1);
    mdio_write_bit(0);

    for (i = 0U; i < 5U; i++)
    {
        mdio_write_bit((phy >> (4U - i)) & 1U);
    }

    for (i = 0U; i < 5U; i++)
    {
        mdio_write_bit((reg >> (4U - i)) & 1U);
    }

    (void)mdio_read_bit();
    (void)mdio_read_bit();

    for (i = 0U; i < 16U; i++)
    {
        data = (uint16_t)((data << 1) | (uint16_t)mdio_read_bit());
    }

    *val = data;
    return 0;
}

int eth_phy_test(void)
{
    uint16_t id1;
    uint16_t id2;
    uint16_t bmsr;
    uint8_t phy;

    mdio_init();

    HAL_GPIO_WritePin(ETH_RST_PORT, ETH_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(10U);
    HAL_GPIO_WritePin(ETH_RST_PORT, ETH_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(50U);

    for (phy = 0U; phy < 32U; phy++)
    {
        if (eth_phy_read(phy, 2U, &id1) != 0)
        {
            continue;
        }

        if ((id1 != 0xFFFFU) && (id1 != 0x0000U))
        {
            if (eth_phy_read(phy, 3U, &id2) != 0)
            {
                continue;
            }
            if (eth_phy_read(phy, 1U, &bmsr) != 0)
            {
                bmsr = 0U;
            }

            console_printf("ETH: PHY at addr %u, ID %04X:%04X, BMSR %04X\n",
                           phy, id1, id2, bmsr);
            console_puts("ETH: MDIO only; RMII REF_CLK is not wired to PA1, "
                         "MAC is not usable in rev A\n");
            return 0;
        }
    }

    console_puts("ETH: no PHY found on MDIO\n");
    return -1;
}
