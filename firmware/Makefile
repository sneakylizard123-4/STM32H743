CROSS ?= $(shell command -v arm-none-eabi-gcc >/dev/null 2>&1 && echo arm-none-eabi- || echo /tmp/xpack-gcc/bin/arm-none-eabi-)

CC      := $(CROSS)gcc
AS      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy
SIZE    := $(CROSS)size

BUILD   := build
TARGET  := $(BUILD)/STM32H743

MCU     := -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard

DEFS    := -DUSE_HAL_DRIVER -DSTM32H743xx
INCLUDES := -Iinc \
            -IDrivers/STM32H7xx_HAL_Driver/Inc \
            -IDrivers/CMSIS/Device/ST/STM32H7xx/Include \
            -IDrivers/CMSIS/Include

CFLAGS  := $(MCU) $(DEFS) $(INCLUDES) -O2 -g3 -Wall -Wextra -Wno-unused-parameter \
           -ffunction-sections -fdata-sections -MMD -MP
ASFLAGS := $(MCU) $(DEFS) $(INCLUDES) -g3
LDFLAGS := $(MCU) -TSTM32H743XIHx_FLASH.ld -Wl,--gc-sections \
           -Wl,-Map=$(TARGET).map -Wl,--print-memory-usage \
           -specs=nano.specs -specs=nosys.specs

HAL_SRC := Drivers/STM32H7xx_HAL_Driver/Src

C_SRCS := \
  src/main.c \
  src/board.c \
  src/clock.c \
  src/console.c \
  src/si5351.c \
  src/fpga.c \
  src/sd.c \
  src/audio.c \
  src/eth_phy.c \
  src/ulpi.c \
  src/dfsd.c \
  src/fpga_bitstream.c \
  $(HAL_SRC)/stm32h7xx_hal.c \
  $(HAL_SRC)/stm32h7xx_hal_cortex.c \
  $(HAL_SRC)/stm32h7xx_hal_rcc.c \
  $(HAL_SRC)/stm32h7xx_hal_rcc_ex.c \
  $(HAL_SRC)/stm32h7xx_hal_gpio.c \
  $(HAL_SRC)/stm32h7xx_hal_i2c.c \
  $(HAL_SRC)/stm32h7xx_hal_i2c_ex.c \
  $(HAL_SRC)/stm32h7xx_hal_spi.c \
  $(HAL_SRC)/stm32h7xx_hal_uart.c \
  $(HAL_SRC)/stm32h7xx_hal_sai.c \
  $(HAL_SRC)/stm32h7xx_hal_sai_ex.c \
  $(HAL_SRC)/stm32h7xx_hal_sd.c \
  $(HAL_SRC)/stm32h7xx_hal_sd_ex.c \
  $(HAL_SRC)/stm32h7xx_hal_dma.c \
  $(HAL_SRC)/stm32h7xx_hal_dma_ex.c \
  $(HAL_SRC)/stm32h7xx_hal_dfsdm.c \
  $(HAL_SRC)/stm32h7xx_hal_dfsdm_ex.c \
  $(HAL_SRC)/stm32h7xx_hal_pwr.c \
  $(HAL_SRC)/stm32h7xx_hal_pwr_ex.c \
  Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/system_stm32h7xx.c

ASM_SRCS := Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h743xx.s

OBJS := $(addprefix $(BUILD)/,$(notdir $(C_SRCS:.c=.o))) \
        $(addprefix $(BUILD)/,$(notdir $(ASM_SRCS:.s=.o)))

vpath %.c src $(HAL_SRC) Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates
vpath %.s Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc

all: $(TARGET).elf $(TARGET).bin $(TARGET).hex

$(BUILD):
	mkdir -p $@

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.s | $(BUILD)
	$(AS) $(ASFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS) STM32H743XIHx_FLASH.ld | $(BUILD)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

bitstream:
	$(MAKE) -C fpga
	python3 tools/embed_bitstream.py

clean:
	rm -rf $(BUILD)

distclean: clean
	$(MAKE) -C fpga clean

-include $(OBJS:.o=.d)

.PHONY: all bitstream clean distclean
