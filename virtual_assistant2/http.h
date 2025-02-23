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
    char header[256];  // Buffer for the HTTP header (small fixed size)
    int header_length; // Total length of the header
    int header_sent;   // Number of header bytes already sent

    const char *body; // Pointer to the JSON body (provided by caller)
    int body_length;  // Length of the JSON body
    int body_sent;    // Number of body bytes already sent
} connection_data_t;

extern volatile bool tcp_request_complete;

static void tcp_client_err_callback(void *arg, err_t err);

static err_t send_next_chunk(struct tcp_pcb *tcpb, connection_data_t *conn_data);

static err_t tcp_client_connected_callback(void *arg, struct tcp_pcb *tcpb, err_t err);

static err_t tcp_client_recv_callback(void *arg, struct tcp_pcb *tcpb, struct pbuf *p, err_t err);

static err_t tcp_client_sent_callback(void *arg, struct tcp_pcb *tcpb, u16_t len);

void tcp_send_audio(const char *audio_data);

#endif