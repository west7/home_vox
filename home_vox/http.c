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

// Variável global que sinaliza a conclusão do envio via TCP.
volatile bool tcp_request_complete = false;

// Buffer para armazenar a resposta da API e flag para indicar quando a resposta está pronta.
char api_response_buffer[512] = {0};
volatile bool api_response_ready = false;

/*
 * Função de callback para tratamento de erros na conexão TCP.
 * Em caso de erro, libera a memória alocada para a conexão.
 */
static void tcp_client_err_callback(void *arg, err_t err)
{
    connection_data_t *conn_data = (connection_data_t *)arg;
    if (conn_data)
    {
        free((void *)conn_data->body);
        free(conn_data);
    }
}
/*
 * Função que envia o próximo "chunk" (pedaço) de dados através da conexão TCP.
 * Primeiro, envia a parte do header, e depois o corpo (body) da mensagem.
 */
static err_t send_next_chunk(struct tcp_pcb *tcpb, connection_data_t *conn_data)
{
    err_t err;
    int chunk_size = 0;

    // Se o header ainda não foi completamente enviado
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
            // Se o header foi completamente enviado, atualiza o estado para enviar o corpo
            if (conn_data->header_sent >= conn_data->header_length)
            {
                conn_data->state = STATE_SEND_BODY;
            }
            return err;
        }
    }

    // Se o header já foi enviado, envia o corpo em "chunks"
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
            // Se o corpo foi completamente enviado, marca o estado como concluído
            if (conn_data->body_sent >= conn_data->body_length)
            {
                conn_data->state = STATE_SEND_DONE;
            }
            return err;
        }
    }
    return ERR_OK;
}

/*
 * Função de callback chamada quando um "chunk" de dados é enviado e reconhecido.
 * Se ainda restarem dados para enviar, chama send_next_chunk().
 */
static err_t tcp_client_sent_callback(void *arg, struct tcp_pcb *tcpb, u16_t len)
{
    connection_data_t *conn_data = (connection_data_t *)arg;

    // Se o envio ainda não terminou, continua enviando o próximo chunk
    if (conn_data->state != STATE_SEND_DONE)
    {
        return send_next_chunk(tcpb, conn_data);
    }
    else
    {
        printf("All data has been sent.\n");
    }
    return ERR_OK;
}

/*
 * Função de callback executada quando a conexão TCP é estabelecida com sucesso.
 * Configura o header HTTP com os dados necessários (incluindo o Content-Length) e inicia o 
 * envio dos dados.
 */
static err_t tcp_client_connected_callback(void *arg, struct tcp_pcb *tcpb, err_t err)
{
    if (err != ERR_OK)
        return err;

    connection_data_t *conn_data = (connection_data_t *)arg;
    printf("Connected to server\n");

    // Calcula o tamanho do corpo do JSON (payload)
    conn_data->body_length = strlen(conn_data->body);

    // Constrói o header HTTP com informações necessárias, como endpoint e Content-Length
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

    // Inicia o envio do header (e posteriormente do corpo) através do TCP
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

/*
 * Função de callback para receber dados da API via TCP.
 * Quando a conexão é fechada pelo servidor, sinaliza a conclusão da transferência,
 * copia a resposta para um buffer global e libera a memória da conexão.
 */
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
    // Copia a resposta para o buffer global, garantindo que esteja null-terminated
    int len = p->len < sizeof(api_response_buffer) - 1 ? p->len : sizeof(api_response_buffer) - 1;
    memcpy(api_response_buffer, p->payload, len);
    api_response_buffer[len] = '\0'; // null terminator
    api_response_ready = true;

    printf("Response: %.*s\n", p->len, (char *)p->payload);
    tcp_recved(tcpb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

/*
 * Função principal para enviar o áudio (em formato JSON) para a API.
 * Cria uma nova conexão TCP, configura os callbacks para recebimento, envio e erros,
 * e inicia o processo de conexão e transmissão dos dados.
 */
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

    // Transfere a ownership do JSON para a estrutura de conexão (por isso o json_payload não 
    // pode ser liberado no loop principal)
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
