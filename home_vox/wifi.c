#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "wifi.h"

void connect_wifi(const char *ssid, const char *password)
{
    if (cyw43_arch_init())
    {
        printf("Error initializing Wi-Fi\n");
        return;
    }

    cyw43_arch_enable_sta_mode();
    printf("Connecting to WiFi: %s...\n", ssid);

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        printf("Failed to connect to WiFi\n");
        return;
    }
    else
    {
        printf("Connected to WiFi\n");
        uint8_t *ip = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
        printf("IP Address: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    }
}