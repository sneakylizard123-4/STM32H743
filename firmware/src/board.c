#include "board.h"

#include "stm32h7xx_hal.h"

static void board_dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->LAR = 0xC5ACCE55U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;
}

void board_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    board_dwt_init();

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOJ_CLK_ENABLE();

    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    gpio.Pin = GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &gpio);

    gpio.Pin = GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOF, &gpio);

    gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    HAL_GPIO_Init(GPIOG, &gpio);

    gpio.Pin = GPIO_PIN_4;
    HAL_GPIO_Init(GPIOJ, &gpio);

    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOF,
                      GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15,
                      GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_4, GPIO_PIN_SET);

    board_delay_us(20000);
}

void board_led_set(board_led_t led, int on)
{
    GPIO_TypeDef *port;
    uint16_t pin;

    switch (led)
    {
    case BOARD_LED_MCU_ACT:
        port = GPIOE;
        pin = GPIO_PIN_15;
        break;
    case BOARD_LED_MCU_FAULT:
        port = GPIOF;
        pin = GPIO_PIN_15;
        break;
    case BOARD_LED_FPGA_ACT:
        port = GPIOF;
        pin = GPIO_PIN_14;
        break;
    case BOARD_LED_FPGA_FAULT:
        port = GPIOF;
        pin = GPIO_PIN_12;
        break;
    default:
        return;
    }

    HAL_GPIO_WritePin(port, pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void board_led_toggle(board_led_t led)
{
    GPIO_TypeDef *port;
    uint16_t pin;

    switch (led)
    {
    case BOARD_LED_MCU_ACT:
        port = GPIOE;
        pin = GPIO_PIN_15;
        break;
    case BOARD_LED_MCU_FAULT:
        port = GPIOF;
        pin = GPIO_PIN_15;
        break;
    case BOARD_LED_FPGA_ACT:
        port = GPIOF;
        pin = GPIO_PIN_14;
        break;
    case BOARD_LED_FPGA_FAULT:
        port = GPIOF;
        pin = GPIO_PIN_12;
        break;
    default:
        return;
    }

    HAL_GPIO_TogglePin(port, pin);
}

void board_delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (HAL_RCC_GetSysClockFreq() / 1000000U);

    while ((DWT->CYCCNT - start) < ticks)
    {
    }
}

uint32_t board_millis(void)
{
    return HAL_GetTick();
}

void Error_Handler(void)
{
    __disable_irq();
    board_led_set(BOARD_LED_MCU_FAULT, 1);
    board_led_set(BOARD_LED_FPGA_FAULT, 1);
    for (;;)
    {
        __NOP();
    }
}
