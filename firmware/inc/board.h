#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

typedef enum
{
    BOARD_LED_MCU_ACT = 0,
    BOARD_LED_MCU_FAULT,
    BOARD_LED_FPGA_ACT,
    BOARD_LED_FPGA_FAULT
} board_led_t;

void board_init(void);
void board_led_set(board_led_t led, int on);
void board_led_toggle(board_led_t led);
void board_delay_us(uint32_t us);
uint32_t board_millis(void);
void Error_Handler(void);

#endif
