#pragma once
#include <stdint.h>

void audio_capture_init();
int audio_capture_read(int16_t *buffer, int samples);