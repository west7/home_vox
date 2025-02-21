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

static err_t tcp_client_connected_callback(void *arg, struct tcp_pcb *tcpb, err_t err);

static err_t tcp_client_recv_callback(void *arg, struct tcp_pcb *tcpb, struct pbuf *p, err_t err);

void tcp_send_audio(const char *audio_data);

#endif