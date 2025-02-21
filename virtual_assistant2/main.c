#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "wifi.h"
#include "http.h"

#define R_LED 13
#define BUZZER 10
#define WIFI_SSID "Boboy_2.4GHz"
#define WIFI_PASS "13zb0276"


const char* record_audio()
{
    // Simulate recording audio
    return "lights on";
}

int main()
{
    stdio_init_all();

    // LED
    gpio_init(R_LED);
    gpio_set_dir(R_LED, GPIO_OUT);

    // BUZZER
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);

    //MIC

    connect_wifi(WIFI_SSID, WIFI_PASS);


    while (true) {

        // Simulate recording audio in the main loop
        const char *audio_data = record_audio();

        // Send the recorded "audio" data via TCP to your Flask API
        tcp_send_audio(audio_data);

        // Wait a bit before sending the next message
        sleep_ms(5000);
    }
}
