#include "audio_capture.h"
#include <Arduino.h>
#include <driver/adc.h>

#define SAMPLE_RATE 16000 // ===== Hz, O MESMO QUE NO GATEWAY =====
#define SAMPLE_PERIOD (1000000 / SAMPLE_RATE)

static uint32_t next_sample;

static float dc = 2048;
static float hp_y = 0;
static float hp_x_prev = 0;

void audio_capture_init()
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // ===== GPIO 34 =====

    next_sample = micros();
}

int audio_capture_read(int16_t *buffer, int samples)
{
    for(int i=0;i<samples;i++)
    {
        while(micros() < next_sample);
        next_sample += SAMPLE_PERIOD;

        int raw = adc1_get_raw(ADC1_CHANNEL_6);

        dc = dc * 0.999 + raw * 0.001;
        float audio = raw - dc;

        float alpha = 0.98;
        hp_y = alpha * (hp_y + audio - hp_x_prev);
        hp_x_prev = audio;

        buffer[i] = (int16_t)hp_y;
    }

    return samples;
}