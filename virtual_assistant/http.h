#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool send_audio(const char *url, const char *audio_data, char *response, size_t response_size);

char *process_response(char *response);

#endif