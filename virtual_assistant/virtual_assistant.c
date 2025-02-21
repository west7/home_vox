#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "wifi.h"


#define R_LED 13
#define BUZZER 10
#define WIFI_SSID "Boboy_2.4GHz"
#define WIFI_PASS "13zb0276"


const char *record_audio()
{
    printf("Recording audio...\n");
    return "acender led";
}

int main ()
{
    stdio_init_all();

    gpio_init(R_LED);
    gpio_set_dir(R_LED, GPIO_OUT);

    sleep_ms(5000);
    connect_wifi(WIFI_SSID, WIFI_PASS);
    sleep_ms(1000);
    
    while (true)
    {
        printf("Listening...\n");

       /*  const char *audio_data = record_audio();
        char response[1024];        

        if (send_audio("http://localhost:5000/upload", audio_data, response, sizeof(response)))
        {
            process_response(response);
        }
        else
        {
            printf("Failed to send audio\n");
        } */
        sleep_ms(5000);

    }
}
