#pragma once
#include <stdint.h>

void wifi_stream_init();
void wifi_stream_send(int16_t *buffer, int samples);