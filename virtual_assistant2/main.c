#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "wifi.h"
#include "http.h"
#include "mic.h"
#include "base64.h"

#define R_LED 13
#define B_LED 12
#define G_LED 11
#define BUZZER 10
#define BTN_B 6

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
    stdio_init_all();

    // LED
    gpio_init(R_LED);
    gpio_set_dir(R_LED, GPIO_OUT);

    gpio_init(G_LED);
    gpio_set_dir(G_LED, GPIO_OUT);

    gpio_init(B_LED);
    gpio_set_dir(B_LED, GPIO_OUT);

    // BUZZER
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);

    // BUTTON B
    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);

    // ADC and DMA setup
    adc_dma_setup();
    sleep_ms(100);

    // Connect to Wi-Fi
    connect_wifi(WIFI_SSID, WIFI_PASS);

    bool prev_btn_state = true;
    uint32_t recording_start = 0;
    uint16_t raw_audio[SAMPLES];
    int16_t pcm_audio[SAMPLES * 2];
    bool buffer_ready = false;

    gpio_put(G_LED, 1);
    sleep_ms(1500);
    gpio_put(G_LED, 0);

    while (true)
    {
        bool current_btn_state = gpio_get(BTN_B);

        switch (current_state)
        {
        case IDLE:
            //  Write "Press B to record" in the OLED

            if (!current_btn_state && prev_btn_state)
            {
                current_state = RECORDING;
                recording_start = time_us_32();
                //  Write "Recording..." in the OLED
                //  Turn off all LEDs and enable green for recording

                record_mic(raw_audio);
                pcm_convert_audio(pcm_audio, raw_audio);
            }
            break;

        case RECORDING:
            if ((uint64_t)(time_us_32() - recording_start) >=
                ((uint64_t)SAMPLES * 1000000ULL) / 8000ULL)
            {
                current_state = PROCESSING;
                buffer_ready = true;
            }
            break;
        case PROCESSING:
            if (buffer_ready)
            {
                current_state = SENDING;
            }
            break;

        case SENDING:

            size_t base64_len;
            char *base64_audio = base64_encode((uint8_t *)pcm_audio, SAMPLES, &base64_len);
            if (base64_audio)
            {

                char *json_payload = malloc(base64_len + 256);
                if (json_payload)
                {
                    snprintf(json_payload, base64_len + 256,
                             "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":8000,"
                             "\"languageCode\":\"pt-BR\"},\"audio\":{\"content\":\"%s\"}}",
                             base64_audio);

                    tcp_send_audio(json_payload);
                    // free(json_payload);
                }
                free(base64_audio);
                while (!tcp_request_complete)
                {
                    sleep_ms(100);
                }
            }
            tcp_request_complete = false;
            buffer_ready = false;
            current_state = EXECUTING;
            break;

        case EXECUTING:
            if (api_response_ready)
            {
                // Look for the "command" field in the response.
                char *command_start = strstr(api_response_buffer, "\"command\":\"");
                if (command_start)
                {
                    command_start += strlen("\"command\":\""); // Move pointer to start of command value.
                    char *command_end = strchr(command_start, '\"');
                    if (command_end)
                    {
                        *command_end = '\0'; // Terminate the command string.

                        // Check for LED_OFF command first.
                        if (strcasecmp(command_start, "LED_OFF") == 0)
                        {
                            gpio_put(R_LED, 0);
                            gpio_put(G_LED, 0);
                            gpio_put(B_LED, 0);
                        }
                        else
                        {
                            // Assume the command is in the form "SET_COLOR <color>"
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
                api_response_ready = false;
            }

            current_state = IDLE;
            break;
        }
        prev_btn_state = current_btn_state;
    }
}
