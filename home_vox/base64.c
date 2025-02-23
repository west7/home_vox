#include "base64.h"
#include <stdlib.h>
#include <string.h>

static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *base64_encode(const uint8_t *pcm_buffer, size_t input_length, size_t *output_length)
{
    const uint8_t *byte_buffer = (const uint8_t *)pcm_buffer;
    size_t byte_len = input_length * sizeof(int16_t);

    // Calculate base64 output length (including padding)
    *output_length = 4 * ((byte_len + 2) / 3);
    char *b64_output = malloc(*output_length + 1); // +1 for null terminator
    if (!b64_output)
        return NULL;

    size_t i, j;
    for (i = 0, j = 0; i < byte_len; i += 3, j += 4)
    {
        uint32_t triple = (i + 2 < byte_len) ? (byte_buffer[i] << 16) | (byte_buffer[i + 1] << 8) | byte_buffer[i + 2] : (i + 1 < byte_len) ? (byte_buffer[i] << 16) | (byte_buffer[i + 1] << 8)
                                                                                                                                            : (byte_buffer[i] << 16);

        b64_output[j] = b64_table[(triple >> 18) & 0x3F];
        b64_output[j + 1] = b64_table[(triple >> 12) & 0x3F];
        b64_output[j + 2] = (i + 1 < byte_len) ? b64_table[(triple >> 6) & 0x3F] : '=';
        b64_output[j + 3] = (i + 2 < byte_len) ? b64_table[triple & 0x3F] : '=';
    }

    b64_output[*output_length] = '\0'; // Null-terminate
    return b64_output;
}