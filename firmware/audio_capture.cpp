#include "audio_capture.h"
#include <Arduino.h>
#include <driver/adc.h>

// Taxa de amostragem do sistema de áudio (16 kHz)
// Deve ser a mesma utilizada pelo gateway receptor
#define SAMPLE_RATE 16000 // ===== Hz, O MESMO QUE NO GATEWAY =====

// Período entre amostras em microssegundos
// 1 segundo = 1.000.000 µs
#define SAMPLE_PERIOD (1000000 / SAMPLE_RATE)

// Variável que armazena o instante da próxima amostragem
static uint32_t next_sample;

// Variáveis usadas no processamento do sinal
// dc: estimativa do offset DC do ADC
// hp_y e hp_x_prev: estados do filtro passa-alta
static float dc = 2048;
static float hp_y = 0;
static float hp_x_prev = 0;


// Inicialização do sistema de captura de áudio
void audio_capture_init()
{
    // Configura resolução do ADC para 12 bits
    adc1_config_width(ADC_WIDTH_BIT_12);

    // Configura atenuação do canal ADC
    // ADC1_CHANNEL_6 corresponde ao GPIO34 no ESP32
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // ===== GPIO 34 =====

    // Inicializa o temporizador de amostragem
    next_sample = micros();
}


// Função que captura um bloco de amostras de áudio
// buffer: ponteiro para o vetor onde os samples serão armazenados
// samples: número de amostras a capturar
int audio_capture_read(int16_t *buffer, int samples)
{
    for(int i=0;i<samples;i++)
    {
        // Aguarda até o instante programado da próxima amostragem
        // Isso garante uma taxa de amostragem estável
        while(micros() < next_sample);

        // Agenda o instante da próxima amostra
        next_sample += SAMPLE_PERIOD;

        // Lê o valor bruto do ADC
        int raw = adc1_get_raw(ADC1_CHANNEL_6);

        // ===== Remoção de offset DC =====
        // Calcula lentamente a média do sinal (filtro passa-baixa)
        dc = dc * 0.999 + raw * 0.001;

        // Remove o componente DC do sinal
        float audio = raw - dc;

        // ===== Filtro passa-alta simples =====
        // Reduz ruídos de baixa frequência e variações lentas
        float alpha = 0.98;
        hp_y = alpha * (hp_y + audio - hp_x_prev);

        // Armazena a última entrada do filtro
        hp_x_prev = audio;

        // Converte o valor filtrado para 16 bits
        // e grava no buffer de saída
        buffer[i] = (int16_t)hp_y;
    }

    // Retorna o número de amostras capturadas
    return samples;
}