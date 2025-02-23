#include "mic.h"

uint dma_channel;
dma_channel_config dma_cfg;

static inline int16_t clamp(int32_t value)
{
    return (value > 32767) ? 32767 : (value < -32768) ? -32768
                                                      : (int16_t)value;
}

void adc_dma_setup()
{
    adc_init();
    adc_gpio_init(MIC_PIN);
    adc_select_input(MIC_CHANNEL);
    adc_fifo_setup(
        true,
        true,
        1,
        false,
        false);

    adc_set_clkdiv(ADC_CLK_DIV);

    dma_channel = dma_claim_unused_channel(true);
    dma_cfg = dma_channel_get_default_config(dma_channel);

    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);
    channel_config_set_dreq(&dma_cfg, DREQ_ADC);
}

/* void adc_dma_reset()
{
    dma_channel_abort(dma_channel);
    adc_fifo_drain();
} */

void record_mic(uint16_t *adc_buf)
{
    adc_fifo_drain();
    adc_run(false);

    dma_channel_configure(dma_channel, &dma_cfg,
                          adc_buf,
                          &(adc_hw->fifo),
                          SAMPLES,
                          true);

    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_channel);

    adc_run(false);
    dma_channel_abort(dma_channel);
}

void pcm_convert_audio(int16_t *pcm_buf, uint16_t *adc_buf)
{
    for (size_t i = 0; i < SAMPLES; i++)
    {
        // Mask to 12 bits and center around zero
        int32_t pcm_value = (adc_buf[i] & 0x0FFF) - ADC_BIAS;
        // Scale to 16-bit (no clamp needed for 12-bit ADC)
        pcm_buf[i] = (int16_t)(pcm_value << 4);
    }
}