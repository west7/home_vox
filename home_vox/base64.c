#include "base64.h"
#include <stdlib.h>
#include <string.h>

// Tabela de caracteres utilizada na codificação Base64.
static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
 * Codifica um buffer de áudio (em PCM) para o formato Base64.
 * pcm_buffer: Buffer de entrada com os dados em PCM.
 * input_length: Número de amostras (elementos) no buffer.
 * output_length: Retorno do tamanho da string codificada.
 * Retorna um ponteiro para a string codificada em Base64, ou NULL se ocorrer erro.
 */
char *base64_encode(const uint8_t *pcm_buffer, size_t input_length, size_t *output_length)
{
    // Interpreta o buffer de entrada como uma sequência de bytes.
    const uint8_t *byte_buffer = (const uint8_t *)pcm_buffer;
    // Calcula o tamanho total em bytes, considerando que cada amostra ocupa 2 bytes (int16_t).
    size_t byte_len = input_length * sizeof(int16_t);

    // Calcula o tamanho da saída Base64, incluindo o padding, conforme especificação.
    *output_length = 4 * ((byte_len + 2) / 3);
    // Aloca memória para a string de saída, adicionando 1 byte para o terminador nulo.
    char *b64_output = malloc(*output_length + 1); // +1 para o null terminator
    if (!b64_output)
        return NULL;

    size_t i, j;
    // Processa os dados em blocos de 3 bytes e os converte em 4 caracteres Base64.
    for (i = 0, j = 0; i < byte_len; i += 3, j += 4)
    {
        // Combina até 3 bytes em um número de 24 bits.
        uint32_t triple = (i + 2 < byte_len) ? (byte_buffer[i] << 16) | (byte_buffer[i + 1] << 8) | byte_buffer[i + 2] : (i + 1 < byte_len) ? (byte_buffer[i] << 16) | (byte_buffer[i + 1] << 8) : (byte_buffer[i] << 16);

        // Mapeia os 24 bits em 4 caracteres usando a tabela Base64.
        b64_output[j] = b64_table[(triple >> 18) & 0x3F];
        b64_output[j + 1] = b64_table[(triple >> 12) & 0x3F];
        b64_output[j + 2] = (i + 1 < byte_len) ? b64_table[(triple >> 6) & 0x3F] : '=';
        b64_output[j + 3] = (i + 2 < byte_len) ? b64_table[triple & 0x3F] : '=';
    }

    b64_output[*output_length] = '\0'; // null terminator
    return b64_output;
}