#ifndef SI5351_H
#define SI5351_H

#include "stm32h7xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

int si5351_init(void);
int si5351_read_reg(uint8_t reg, uint8_t *val);
int si5351_write_reg(uint8_t reg, uint8_t val);
int si5351_locked(void);

#endif
