#ifndef HTTP_H
#define HTTP_H

#include <stdbool.h>
#include <stdlib.h>

#include "pico/cyw43_arch.h"

// LWIP headers in correct order
#include "lwip/tcp.h"
#include "lwip/pbuf.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"

// Variáveis globais para controle e recuperação da resposta da API
extern char api_response_buffer[];
extern volatile bool api_response_ready;
typedef enum
{
    STATE_SEND_HEADER,
    STATE_SEND_BODY,
    STATE_SEND_DONE
} send_state_t;
typedef struct
{
    send_state_t state;
    char header[256];  // Buffer para o header HTTP
    int header_length; // Tamanho do header
    int header_sent;   // Número de bytes do header já enviados

    const char *body; // Ponteiro para o JSON
    int body_length;  // Tamanho do JSON
    int body_sent;    // Número de bytes do JSON já enviados
} connection_data_t;

// Variável global para controle da conexão TCP
extern volatile bool tcp_request_complete;

// Funções de callback do LWIP 
static void tcp_client_err_callback(void *arg, err_t err);

static err_t send_next_chunk(struct tcp_pcb *tcpb, connection_data_t *conn_data);

static err_t tcp_client_connected_callback(void *arg, struct tcp_pcb *tcpb, err_t err);

static err_t tcp_client_recv_callback(void *arg, struct tcp_pcb *tcpb, struct pbuf *p, err_t err);

static err_t tcp_client_sent_callback(void *arg, struct tcp_pcb *tcpb, u16_t len);

// Função principal que estabele conexão TCP e envio de requisição HTTP
void tcp_send_audio(const char *audio_data);

#endif