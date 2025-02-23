#include "mic.h"

// Variáveis globais para configuração do canal DMA.
uint dma_channel;
dma_channel_config dma_cfg;

/*
 * Limita um valor de 32 bits ao intervalo de um inteiro de 16 bits.
 * Se o valor ultrapassar 32767, retorna 32767; se for inferior a -32768, retorna -32768;
 * caso contrário, converte o valor para int16_t.
 */
static inline int16_t clamp(int32_t value)
{
    return (value > 32767) ? 32767 : (value < -32768) ? -32768
                                                      : (int16_t)value;
}

/*
 * Configura o ADC e o DMA para a captura do áudio do microfone.
 * Inicializa o ADC e configura o pino do microfone.
 * Seleciona o canal ADC apropriado.
 * Configura a FIFO do ADC para armazenar temporariamente as amostras.
 * Ajusta a taxa de amostragem com o divisor de clock.
 * Reivindica um canal DMA e configura suas propriedades:
 *    * Tamanho de transferência de 16 bits.
 *    * Incremento de leitura desativado (pois o ADC lê sempre do mesmo registrador).
 *    * Incremento de escrita ativado (para armazenar os dados sequencialmente no buffer).
 *    * Configura o DREQ para sincronização com o ADC.
 */
void adc_dma_setup()
{
    adc_init();                    // Inicializa o ADC.
    adc_gpio_init(MIC_PIN);        // Configura o pino do microfone.
    adc_select_input(MIC_CHANNEL); // Seleciona o canal ADC apropriado (ADC02).
    adc_fifo_setup(
        true,                      // Habilita o uso da FIFO.
        true,                      // Habilita o DMA para a FIFO.
        1,                         // Define o nível de alerta para uma amostra.
        false,                     // Não desabilita o burst mode.
        false);                    // Não usa fluxo de dados dinâmico.

    adc_set_clkdiv(ADC_CLK_DIV); // Ajusta a taxa de amostragem com o divisor de clock.

    // Configuração do canal DMA.
    dma_channel = dma_claim_unused_channel(true);
    dma_cfg = dma_channel_get_default_config(dma_channel);

    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);
    channel_config_set_dreq(&dma_cfg, DREQ_ADC);
}

/*
 * Captura os dados de áudio do microfone utilizando ADC e DMA.
 * Esvazia a FIFO do ADC para remover dados residuais.
 * Desativa o ADC para configurar a transferência.
 * Configura o canal DMA para transferir 'SAMPLES' de dados do ADC para o buffer adc_buf.
 * Ativa o ADC para iniciar a captura e espera, de forma bloqueante, a conclusão da transferência via DMA.
 * Após a captura, desativa o ADC e aborta o canal DMA para liberar recursos.
 */
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

/*
 * pcm_convert_audio()
 * Converte os dados brutos do ADC (12 bits) para o formato PCM de 16 bits.
 * Para cada amostra:
 * - Aplica uma máscara para extrair os 12 bits significativos.
 * - Subtrai o ADC_BIAS para centralizar os valores em torno de zero.
 * - Realiza um deslocamento à esquerda de 4 bits para escalar o valor para 16 bits.
 */
void pcm_convert_audio(int16_t *pcm_buf, uint16_t *adc_buf)
{
    for (size_t i = 0; i < SAMPLES; i++)
    {
        // Aplica máscara de 12 bits e centraliza o valor subtraindo o ADC_BIAS
        int32_t pcm_value = (adc_buf[i] & 0x0FFF) - ADC_BIAS;
        // Ajusta a escala para 16 bits (deslocamento à esquerda de 4 bits)
        pcm_buf[i] = (int16_t)(pcm_value << 4);
    }
}