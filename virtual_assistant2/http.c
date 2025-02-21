#include "http.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "pico/cyw43_arch.h"

// LWIP headers in correct order
#include "lwip/tcp.h"
#include "lwip/pbuf.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"

#define SERVER_IP "192.168.1.111"
#define SERVER_PORT 5000
#define API_ENDPOINT "/speech"
#define API_TEST_ENDPOINT "/test"

typedef struct
{
    const char *audio_data;
} connection_data_t;

static err_t tcp_client_connected_callback(void *arg, struct tcp_pcb *tcpb, err_t err)
{
    if (err != ERR_OK)
        return err;

    connection_data_t *conn_data = (connection_data_t *)arg;
    const char *data = conn_data->audio_data;
    int content_length = strlen(data);

    char request[512];
    sniprintf(request, sizeof(request),
              "POST %s HTTP/1.1\r\n"
              "Host: %s\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: %d\r\n"
              "\r\n"
              "%s",
              API_ENDPOINT, SERVER_IP, content_length, data);

    tcp_write(tcpb, request, strlen(request), TCP_WRITE_FLAG_COPY);

    free(conn_data);

    return ERR_OK;
}

static err_t tcp_client_recv_callback(void *arg, struct tcp_pcb *tcpb, struct pbuf *p, err_t err)
{   
    if (!p)
    {
        printf("Connection closed\n");
        tcp_close(tcpb);
        return ERR_OK;
    }

    printf("Response: %.*s\n", p->len, (char *)p->payload);
    tcp_recved(tcpb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}



void tcp_send_audio(const char *audio_data)
{
    struct tcp_pcb *tcp_client = tcp_new();
    if (!tcp_client)
    {
        printf("Error creating new TCP PCB.\n");
        return;
    }

    connection_data_t *conn_data = malloc(sizeof(connection_data_t));
    if (!conn_data)
    {
        printf("Memory allocation error.\n");
        tcp_abort(tcp_client);
        return;
    }
    conn_data->audio_data = audio_data;

    ip_addr_t server_ip;
    ipaddr_aton(SERVER_IP, &server_ip);

    tcp_arg(tcp_client, conn_data);
    tcp_recv(tcp_client, tcp_client_recv_callback);
    err_t err = tcp_connect(tcp_client, &server_ip, SERVER_PORT, tcp_client_connected_callback);

    if (err != ERR_OK)
    {
        printf("Error connecting to server: %d\n", err);
        tcp_abort(tcp_client);
        return;
    }
}




/* bool send_audio(const char *url, const char *audio_data, char *response, size_t response_size)
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
} */