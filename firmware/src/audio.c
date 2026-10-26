#include "audio.h"

#include "console.h"
#include "stm32h7xx_hal.h"

static SAI_HandleTypeDef hsai2;

static const int16_t sine_1k[48] = {
    0, 2611, 5176, 7654, 10000, 12175, 14142, 15867,
    17321, 18478, 19319, 19829, 20000, 19829, 19319, 18478,
    17321, 15867, 14142, 12175, 10000, 7654, 5176, 2611,
    0, -2611, -5176, -7654, -10000, -12175, -14142, -15867,
    -17321, -18478, -19319, -19829, -20000, -19829, -19319, -18478,
    -17321, -15867, -14142, -12175, -10000, -7654, -5176, -2611
};

void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai)
{
    GPIO_InitTypeDef gpio = {0};

    if (hsai->Instance != SAI2_Block_B)
    {
        return;
    }

    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_SAI2_CLK_ENABLE();

    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF10_SAI2;

    gpio.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    HAL_GPIO_Init(GPIOG, &gpio);

    gpio.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    HAL_GPIO_Init(GPIOH, &gpio);
}

void HAL_SAI_MspDeInit(SAI_HandleTypeDef *hsai)
{
    if (hsai->Instance != SAI2_Block_B)
    {
        return;
    }

    __HAL_RCC_SAI2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_9 | GPIO_PIN_10);
    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_2 | GPIO_PIN_3);
}

int audio_test_tone(void)
{
    int16_t buf[512];
    uint32_t frame;
    uint32_t chunk;
    uint32_t phase = 0U;

    hsai2.Instance = SAI2_Block_B;
    hsai2.Init.AudioMode = SAI_MODEMASTER_TX;
    hsai2.Init.Synchro = SAI_ASYNCHRONOUS;
    hsai2.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
    hsai2.Init.MckOutput = SAI_MCK_OUTPUT_ENABLE;
    hsai2.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;
    hsai2.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
    hsai2.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
    hsai2.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
    hsai2.Init.Mckdiv = 0U;
    hsai2.Init.MckOverSampling = SAI_MCK_OVERSAMPLING_DISABLE;
    hsai2.Init.MonoStereoMode = SAI_STEREOMODE;
    hsai2.Init.CompandingMode = SAI_NOCOMPANDING;
    hsai2.Init.TriState = SAI_OUTPUT_NOTRELEASED;
    hsai2.Init.PdmInit.Activation = DISABLE;
    hsai2.Init.PdmInit.MicPairsNbr = 1U;
    hsai2.Init.PdmInit.ClockEnable = 0U;

    if (HAL_SAI_InitProtocol(&hsai2,
                             SAI_I2S_STANDARD,
                             SAI_PROTOCOL_DATASIZE_16BIT,
                             2U) != HAL_OK)
    {
        console_puts("SAI2B: init failed\n");
        return -1;
    }

    for (frame = 0U; frame < 24064U; frame += 256U)
    {
        for (chunk = 0U; chunk < 256U; chunk++)
        {
            int16_t sample = sine_1k[phase];

            buf[chunk * 2U] = sample;
            buf[(chunk * 2U) + 1U] = sample;

            phase++;
            if (phase >= 48U)
            {
                phase = 0U;
            }
        }

        if (HAL_SAI_Transmit(&hsai2,
                             (uint8_t *)buf,
                             sizeof(buf) / sizeof(uint16_t),
                             100U) != HAL_OK)
        {
            (void)HAL_SAI_Abort(&hsai2);
            console_puts("SAI2B: transmit failed\n");
            return -1;
        }
    }

    (void)HAL_SAI_Abort(&hsai2);
    console_puts("SAI2B: 1 kHz stereo test tone sent\n");
    return 0;
}
