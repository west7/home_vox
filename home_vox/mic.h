#ifndef MIC_H
#define MIC_H

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

// Define o canal do microfone utilizado no ADC.
#define MIC_CHANNEL 2
// Define o pino do microfone.
#define MIC_PIN (26 + MIC_CHANNEL)

// Define o divisor de clock do ADC para ajustar a taxa de conversão.
// Um divisor de 6000 determina a frequência de amostragem efetiva do ADC.
#define ADC_CLK_DIV 6000
// Define a taxa de amostragem para a captura de áudio (em Hz).
// Aqui, o áudio é capturado a 8000 Hz.
#define SAMPLE_RATE 8000
// Define o número total de amostras a serem capturadas.
// Neste caso, o número de amostras é o dobro da taxa de amostragem (por exemplo, para uma gravação de 2 segundos).
#define SAMPLES (SAMPLE_RATE * 2)
// Define o ADC_BIAS, que é utilizado para centralizar os dados do ADC em torno de 2048.
#define ADC_BIAS 2048

// Funções para captura, conversão de áudio e inicialização dos canais.
void record_mic(uint16_t *adc_buf);
void adc_dma_setup();
void pcm_convert_audio(int16_t *pcm_buf, uint16_t *adc_buf);
static inline int16_t clamp(int32_t value);

#endif