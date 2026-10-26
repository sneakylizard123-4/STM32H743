#include "audio.h"
#include "board.h"
#include "clock.h"
#include "console.h"
#include "dfsd.h"
#include "eth_phy.h"
#include "fpga.h"
#include "sd.h"
#include "si5351.h"
#include "stm32h7xx_hal.h"
#include "ulpi.h"

static void report(const char *name, int rc)
{
    console_printf("%s: %s\n", name, (rc == 0) ? "PASS" : "FAIL");
}

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

int main(void)
{
    int si_ok;
    int fpga_ok;
    int sd_ok;
    int ulpi_ok;
    int mcu_fault = 0;
    int fpga_fault = 0;
    uint32_t last;
    int heartbeat = 0;

    HAL_Init();
    board_init();

    console_init();
    console_puts("\nSTM32H743 v1 firmware\n");
    console_puts("Semihosting console over SWD; no UART header on rev A\n");

    if (clock_init() != 0)
    {
        console_puts("Clock init failed\n");
        Error_Handler();
    }

    console_printf("SYSCLK: %lu Hz, HCLK: %lu Hz, PCLK1: %lu Hz\n",
                   (unsigned long)HAL_RCC_GetSysClockFreq(),
                   (unsigned long)HAL_RCC_GetHCLKFreq(),
                   (unsigned long)HAL_RCC_GetPCLK1Freq());

    si_ok = si5351_init();
    report("Si5351 clocks", si_ok);
    if (si_ok != 0)
    {
        mcu_fault = 1;
    }

    fpga_ok = fpga_load();
    report("FPGA bitstream", fpga_ok);
    if (fpga_ok != 0)
    {
        fpga_fault = 1;
    }

    sd_ok = sd_test();
    console_printf("SD card: %s\n",
                   (sd_ok == 0) ? "PASS" : "absent or unreadable (warning)");

    report("Audio DAC", audio_test_tone());
    report("Ethernet PHY", eth_phy_test());

    if (si_ok == 0)
    {
        ulpi_ok = ulpi_test();
        report("USB3300 ULPI", ulpi_ok);
    }
    else
    {
        console_puts("USB3300 ULPI: SKIP, Si5351 did not start\n");
    }

    (void)dfsd_test();

    board_led_set(BOARD_LED_MCU_FAULT, mcu_fault);
    board_led_set(BOARD_LED_FPGA_FAULT, fpga_fault);

    console_puts("Self-test complete; 1 Hz heartbeat on MCU_ACT\n");

    last = HAL_GetTick();
    for (;;)
    {
        if ((HAL_GetTick() - last) >= 500U)
        {
            last += 500U;
            heartbeat = !heartbeat;
            board_led_set(BOARD_LED_MCU_ACT, heartbeat);
            if (heartbeat)
            {
                board_led_toggle(BOARD_LED_FPGA_ACT);
            }
        }
    }
}
