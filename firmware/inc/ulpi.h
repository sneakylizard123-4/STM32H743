#ifndef ULPI_H
#define ULPI_H

#include <stdint.h>

void ulpi_reset(void);
int ulpi_test(void);
int ulpi_read_reg(uint8_t reg, uint8_t *val);
int ulpi_write_reg(uint8_t reg, uint8_t val);

#endif
