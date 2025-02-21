#include "http.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "pico/cyw43_arch.h"

#define LWIP_TIMEVAL_PRIVATE 0  // Evita redefinição da struct timeval
// LWIP headers in correct order
#include "lwip/dns.h"
#include "lwip/tcp.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwipopts.h"


bool send_audio(const char *url, const char *audio_data, char *response, size_t response_size)
{
    // Create socket with lwIP's socket API
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("Error creating socket: %s\n", errno);
        return false;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = PP_HTONS(5000);

    inet_pton(AF_INET, "192.168.1.100", &server_addr.sin_addr); // Endereço do servidor

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Error connecting to server\n");
        close(sock);
        return false;
    }

    char request[1024];
    snprintf(request, sizeof(request),
             "POST /test HTTP/1.1\r\n"
             "Host: localhost\r\n"
             "Content-Type: audio/wav\r\n"
             "Content-Length: %zu\r\n"
             "\r\n"
             "%s",
             strlen(audio_data), audio_data);

    send(sock, request, strlen(request), 0);

    int received = recv(sock, response, response_size - 1, 0);
    if (received < 0)
    {
        printf("Error in response\n");
        close(sock);
        return false;
    }

    response[received] = '\0';
    printf("Response: %s\n", response);
    close(sock);
    return true;
}

char *process_response(char *response)
{
    printf("Resposta processada: %s\n", response);
}