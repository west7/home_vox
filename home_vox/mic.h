#ifndef MIC_H
#define MIC_H

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

#define MIC_CHANNEL 2
#define MIC_PIN (26 + MIC_CHANNEL)

#define ADC_CLK_DIV 6000
#define SAMPLE_RATE 8000
#define SAMPLES (SAMPLE_RATE * 2)
#define ADC_BIAS 2048

void record_mic(uint16_t *adc_buf);
void adc_dma_setup();
void pcm_convert_audio(int16_t *pcm_buf, uint16_t *adc_buf);
static inline int16_t clamp(int32_t value);

#endif