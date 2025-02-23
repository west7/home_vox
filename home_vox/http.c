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

#define CHUNK_SIZE 1024

volatile bool tcp_request_complete = false;

char api_response_buffer[512] = {0};
volatile bool api_response_ready = false;

static void tcp_client_err_callback(void *arg, err_t err)
{
    connection_data_t *conn_data = (connection_data_t *)arg;
    if (conn_data)
    {
        free((void *)conn_data->body);
        free(conn_data);
    }
}

static err_t send_next_chunk(struct tcp_pcb *tcpb, connection_data_t *conn_data)
{
    err_t err;
    int chunk_size = 0;

    // First, send the header if not done.
    if (conn_data->state == STATE_SEND_HEADER)
    {
        int remaining = conn_data->header_length - conn_data->header_sent;
        chunk_size = (remaining < CHUNK_SIZE) ? remaining : CHUNK_SIZE;
        if (chunk_size > 0)
        {
            err = tcp_write(tcpb, conn_data->header + conn_data->header_sent, chunk_size, TCP_WRITE_FLAG_COPY);
            if (err == ERR_OK)
            {
                conn_data->header_sent += chunk_size;
                tcp_output(tcpb);
            }
            // If header is fully sent, update state.
            if (conn_data->header_sent >= conn_data->header_length)
            {
                conn_data->state = STATE_SEND_BODY;
            }
            return err;
        }
    }

    // Next, send the body in chunks.
    if (conn_data->state == STATE_SEND_BODY)
    {
        int remaining = conn_data->body_length - conn_data->body_sent;
        chunk_size = (remaining < CHUNK_SIZE) ? remaining : CHUNK_SIZE;
        if (chunk_size > 0)
        {
            err = tcp_write(tcpb, conn_data->body + conn_data->body_sent, chunk_size, TCP_WRITE_FLAG_COPY);
            if (err == ERR_OK)
            {
                conn_data->body_sent += chunk_size;
                tcp_output(tcpb);
            }
            // If body is fully sent, mark as done.
            if (conn_data->body_sent >= conn_data->body_length)
            {
                conn_data->state = STATE_SEND_DONE;
            }
            return err;
        }
    }
    return ERR_OK;
}

static err_t tcp_client_sent_callback(void *arg, struct tcp_pcb *tcpb, u16_t len)
{
    connection_data_t *conn_data = (connection_data_t *)arg;

    // If not done sending, send the next chunk.
    if (conn_data->state != STATE_SEND_DONE)
    {
        return send_next_chunk(tcpb, conn_data);
    }
    else
    {
        printf("All data has been sent.\n");
        // Optionally signal completion or free connection data here.
    }
    return ERR_OK;
}

static err_t tcp_client_connected_callback(void *arg, struct tcp_pcb *tcpb, err_t err)
{
    if (err != ERR_OK)
        return err;

    connection_data_t *conn_data = (connection_data_t *)arg;
    printf("Connected to server\n");

    // Compute body length from the provided JSON payload.
    conn_data->body_length = strlen(conn_data->body);

    // Build the HTTP header into our small buffer.
    // The header includes the Content-Length which is the length of the JSON body.
    conn_data->header_length = snprintf(conn_data->header, sizeof(conn_data->header),
                                        "POST %s HTTP/1.1\r\n"
                                        "Host: %s:%d\r\n"
                                        "Content-Type: application/json\r\n"
                                        "Content-Length: %d\r\n"
                                        "\r\n",
                                        API_ENDPOINT, SERVER_IP, SERVER_PORT, conn_data->body_length);
    conn_data->header_sent = 0;
    conn_data->body_sent = 0;
    conn_data->state = STATE_SEND_HEADER;

    //printf("Header prepared (length: %d bytes):\n%s\n", conn_data->header_length, conn_data->header);

    // Begin by sending the header.
    err_t write_err = send_next_chunk(tcpb, conn_data);
    if (write_err != ERR_OK)
    {
        printf("Error writing to server: %d\n", write_err);
        tcp_abort(tcpb);
        free((void *)conn_data->body);
        free(conn_data);
        return write_err;
    }
    return ERR_OK;
}

static err_t tcp_client_recv_callback(void *arg, struct tcp_pcb *tcpb, struct pbuf *p, err_t err)
{
    connection_data_t *conn_data = (connection_data_t *)arg;

    if (!p)
    {
        printf("Connection closed by server\n");
        tcp_request_complete = true;
        tcp_close(tcpb);
        if (conn_data)
        {
            free((void *)conn_data->body);
            free(conn_data);
        }
        return ERR_OK;
    }
    // Copy response to global buffer
    int len = p->len < sizeof(api_response_buffer) - 1 ? p->len : sizeof(api_response_buffer) - 1;
    memcpy(api_response_buffer, p->payload, len);
    api_response_buffer[len] = '\0'; // Null-terminate
    api_response_ready = true;

    printf("Response: %.*s\n", p->len, (char *)p->payload);
    tcp_recved(tcpb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

void tcp_send_audio(const char *json)
{
    printf("Sending audio data...\n");
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


    conn_data->body = json;

    ip_addr_t server_ip;
    ipaddr_aton(SERVER_IP, &server_ip);

    tcp_arg(tcp_client, conn_data);
    tcp_recv(tcp_client, tcp_client_recv_callback);
    tcp_sent(tcp_client, tcp_client_sent_callback);
    tcp_err(tcp_client, tcp_client_err_callback);
    err_t err = tcp_connect(tcp_client, &server_ip, SERVER_PORT, tcp_client_connected_callback);

    if (err != ERR_OK)
    {
        printf("Error connecting to server: %d\n", err);
        tcp_abort(tcp_client);
        free((void *)conn_data->body);
        free(conn_data);
        return;
    }
}
