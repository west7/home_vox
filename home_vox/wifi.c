#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "wifi.h"

/*
 * Estabelece a conexão Wi-Fi utilizando as funções da biblioteca cyw43_arch.
 * - ssid: Nome da rede Wi-Fi à qual se deseja conectar.
 * - password: Senha da rede Wi-Fi.
 *
 * O procedimento é o seguinte:
 * 1. Inicializa a arquitetura Wi-Fi. Se a inicialização falhar, imprime uma mensagem de erro e retorna.
 * 2. Habilita o modo "station" para que o dispositivo atue como cliente.
 * 3. Tenta conectar à rede Wi-Fi com um timeout de 10.000 ms, utilizando o método WPA2 AES PSK.
 * 4. Se a conexão falhar, imprime uma mensagem de falha; caso contrário, imprime o endereço IP obtido.
 */
void connect_wifi(const char *ssid, const char *password)
{
    // Inicializa a arquitetura Wi-Fi; se retornar um valor não zero, ocorreu erro.
    if (cyw43_arch_init())
    {
        printf("Error initializing Wi-Fi\n");
        return;
    }

    cyw43_arch_enable_sta_mode();
    printf("Connecting to WiFi: %s...\n", ssid);

    // Tenta conectar à rede com o timeout definido (10.000 ms) e usando autenticação WPA2 AES PSK.
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