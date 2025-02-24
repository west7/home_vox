#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "wifi.h"
#include "http.h"
#include "mic.h"
#include "base64.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

#define R_LED 13
#define B_LED 12
#define G_LED 11
#define BTN_B 6
#define I2C_SDA 14
#define I2C_SCL 15

// Define os estados possíveis do sistema.
typedef enum
{
    IDLE,
    RECORDING,
    PROCESSING,
    SENDING,
    EXECUTING
} state_t;

state_t current_state = IDLE;

int main()
{
    // Inicializa as bibliotecas padrão (stdio, etc.).
    stdio_init_all();

    // Configuração dos pinos dos LEDs (RGB) como saída.
    gpio_init(R_LED);
    gpio_set_dir(R_LED, GPIO_OUT);
    gpio_init(G_LED);
    gpio_set_dir(G_LED, GPIO_OUT);
    gpio_init(B_LED);
    gpio_set_dir(B_LED, GPIO_OUT);

    // Configuração do pino do botão B como entrada com pull-up interno.
    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);

    // Inicializa o display OLED e configura os pinos SDA e SCL.
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init();

    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // zera o display inteiro
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    char *text[] = {
        "  Escutando   ",
        "  Pressione B "};

    int x_pos = 0; // Posição horizontal (coluna)
    int y_pos = 5; // Posição vertical (linha)

    // Configura o ADC e o DMA para a captura de áudio.
    adc_dma_setup();
    sleep_ms(100);

    // Conecta ao Wi-Fi
    connect_wifi(WIFI_SSID, WIFI_PASS);

    // Variáveis para controle do estado do botão e temporização da gravação.
    bool prev_btn_state = true;   // Armazena o estado anterior do botão.
    uint32_t recording_start = 0; // Guarda o instante em que a gravação foi iniciada.

    // Buffers para armazenamento dos dados de áudio:
    // raw_audio: armazena os dados brutos do ADC (16kB para SAMPLES=8000).
    // pcm_audio: armazena os dados convertidos para PCM (32kB para SAMPLES=8000).
    uint16_t raw_audio[SAMPLES]; 
    int16_t pcm_audio[SAMPLES * 2];

    // Flag para indicar que o buffer de áudio está pronto para envio.
    bool buffer_ready = false;

    // Sinal de prontidão: acende o LED verde por 1,5 segundos.
    gpio_put(G_LED, 1);
    sleep_ms(1500);
    gpio_put(G_LED, 0);

    while (true)
    {
        bool current_btn_state = gpio_get(BTN_B);

        // Controle de fluxo baseado no estado atual do sistema.
        switch (current_state)
        {
        case IDLE:
            // Em estado IDLE, aguarda o pressionamento do botão.
            // (Exemplo: exibir mensagem "Press B to record" no OLED)
            ssd1306_draw_string(ssd, x_pos, y_pos, text[1]);
            render_on_display(ssd, &frame_area);
            if (!current_btn_state && prev_btn_state)
            {

                current_state = RECORDING;
                recording_start = time_us_32();
                //  Write "Recording..." in the OLED
                ssd1306_draw_string(ssd, x_pos, y_pos, text[0]);
                render_on_display(ssd, &frame_area);

                // Captura o áudio e converte para PCM.
                record_mic(raw_audio);
                pcm_convert_audio(pcm_audio, raw_audio);
            }
            break;

        case RECORDING:
            // Verifica se o tempo de gravação (baseado em SAMPLES) foi atingido.
            if ((uint64_t)(time_us_32() - recording_start) >=
                ((uint64_t)SAMPLES * 1000000ULL) / 8000ULL)
            {
                current_state = PROCESSING;
                buffer_ready = true;
            }
            break;
        case PROCESSING:
            // Se o buffer está pronto, muda para o estado de envio.
            if (buffer_ready)
            {
                current_state = SENDING;
            }
            break;

        case SENDING:
            // Converte o áudio PCM para Base64 para envio.
            size_t base64_len;
            char *base64_audio = base64_encode((uint8_t *)pcm_audio, SAMPLES, &base64_len);
            if (base64_audio)
            {
                // Cria um payload JSON dinâmico para a requisição HTTP.
                char *json_payload = malloc(base64_len + 256);
                if (json_payload)
                {
                    snprintf(json_payload, base64_len + 256,
                             "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":8000,"
                             "\"languageCode\":\"pt-BR\"},\"audio\":{\"content\":\"%s\"}}",
                             base64_audio);

                    // Envia o áudio codificado para a API via conexão TCP.
                    tcp_send_audio(json_payload);
                }
                free(base64_audio);

                // Aguarda até que a requisição seja completamente enviada.
                while (!tcp_request_complete)
                {
                    sleep_ms(100);
                }
            }
            // Reseta as flags e muda para o estado de execução.
            tcp_request_complete = false;
            buffer_ready = false;
            current_state = EXECUTING;
            break;

        case EXECUTING:
            // Processa a resposta da API e executa a ação correspondente.
            if (api_response_ready)
            {
                // Procura o campo "command" na resposta JSON.
                char *command_start = strstr(api_response_buffer, "\"command\":\"");
                if (command_start)
                {
                    command_start += strlen("\"command\":\""); // Move o ponteiro para o início do comando.
                    char *command_end = strchr(command_start, '\"');
                    if (command_end)
                    {
                        *command_end = '\0'; // Termina a string no final do comando.

                        // Se o comando for "LED_OFF", desliga todos os LEDs.
                        if (strcasecmp(command_start, "LED_OFF") == 0)
                        {
                            gpio_put(R_LED, 0);
                            gpio_put(G_LED, 0);
                            gpio_put(B_LED, 0);
                        }
                        else
                        {
                            // Se o comando for "SET_COLOR <cor>", liga o LED correspondente.
                            char *action = strtok(command_start, " ");
                            char *color = strtok(NULL, " ");
                            if (action && color && strcasecmp(action, "SET_COLOR") == 0)
                            {
                                // Then turn on the specified LED.
                                if (strcasecmp(color, "RED") == 0)
                                {
                                    gpio_put(R_LED, 1);
                                }
                                else if (strcasecmp(color, "GREEN") == 0)
                                {
                                    gpio_put(G_LED, 1);
                                }
                                else if (strcasecmp(color, "BLUE") == 0)
                                {
                                    gpio_put(B_LED, 1);
                                }
                            }
                        }
                    }
                }
                // Reseta a flag de resposta pronta.
                api_response_ready = false;
            }
            // Após a execução, retorna ao estado IDLE.
            current_state = IDLE;
            break;
        }
        // Atualiza o estado anterior do botão para a próxima iteração.
        prev_btn_state = current_btn_state;
    }
}
