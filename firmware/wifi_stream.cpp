#include "wifi_stream.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <string.h>

// Credenciais da rede Wi-Fi utilizada pelo experimento
#define WIFI_SSID "SUA_REDE"
#define WIFI_PASSWORD "SUA_SENHA"

// Endereço IP do gateway que receberá o fluxo de áudio
#define ipgateway 192,168,0,100 // ===== IP DO GATEWAY =====

// Porta UDP utilizada para o envio dos pacotes RTP
#define UDP_PORT 5004 // ===== PORTA DO GATEWAY =====


// ===== Variáveis de controle do protocolo RTP =====

// Número de sequência do pacote RTP
// Incrementado a cada pacote transmitido
static uint16_t rtp_seq = 0;

// Timestamp RTP
// Representa a posição temporal das amostras no fluxo de áudio
static uint32_t rtp_timestamp = 0;

// Identificador da fonte RTP (SSRC)
// Permite identificar a origem do fluxo dentro da sessão
static const uint32_t rtp_ssrc = 0x12345678;


// Endereço IP de destino (gateway)
IPAddress destIP(ipgateway);

// Objeto responsável pela comunicação UDP
WiFiUDP udp;


// Inicializa a conexão Wi-Fi e prepara o envio UDP
void wifi_stream_init() {

    Serial.print("Conectando Wi-Fi...");
    
    // Coloca o ESP32 em modo estação (cliente da rede)
    WiFi.mode(WIFI_STA);

    // Inicia a conexão com a rede configurada
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Aguarda até que a conexão seja estabelecida
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // Informa no terminal que a conexão foi concluída
    Serial.println("Conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // Inicializa o socket UDP local
    udp.begin(UDP_PORT);
}


// Envia um bloco de amostras de áudio através de RTP/UDP
// buffer: vetor contendo as amostras capturadas
// samples: quantidade de amostras presentes no buffer
void wifi_stream_send(int16_t *buffer, int samples) {

    // Cria um pacote contendo:
    // 12 bytes de cabeçalho RTP + dados de áudio
    uint8_t packet[12 + samples * sizeof(int16_t)];

    // ===== Construção do cabeçalho RTP =====

    // Byte 0
    // 0x80 = versão RTP 2, sem padding, sem extensão
    packet[0] = 0x80;      

    // Byte 1
    // Payload type 96 (tipo dinâmico comum para áudio)
    packet[1] = 96;        

    // Número de sequência (16 bits)
    packet[2] = rtp_seq >> 8;
    packet[3] = rtp_seq & 0xFF;

    // Timestamp RTP (32 bits)
    packet[4] = (rtp_timestamp >> 24) & 0xFF;
    packet[5] = (rtp_timestamp >> 16) & 0xFF;
    packet[6] = (rtp_timestamp >> 8) & 0xFF;
    packet[7] = rtp_timestamp & 0xFF;

    // Identificador da fonte (SSRC)
    packet[8]  = (rtp_ssrc >> 24) & 0xFF;
    packet[9]  = (rtp_ssrc >> 16) & 0xFF;
    packet[10] = (rtp_ssrc >> 8) & 0xFF;
    packet[11] = rtp_ssrc & 0xFF;


    // ===== Inserção dos dados de áudio =====

    // Copia o conteúdo do buffer de amostras para o pacote
    // logo após o cabeçalho RTP
    memcpy(packet + 12, buffer, samples * sizeof(int16_t));


    // ===== Envio do pacote pela rede =====

    // Inicia o pacote UDP com destino ao gateway
    udp.beginPacket(destIP, UDP_PORT);

    // Escreve o pacote completo (RTP + áudio)
    udp.write(packet, sizeof(packet));

    // Finaliza e envia o pacote
    udp.endPacket();


    // ===== Atualização dos campos RTP =====

    // Incrementa o número de sequência do pacote
    rtp_seq++;

    // Avança o timestamp de acordo com o número de amostras enviadas
    rtp_timestamp += samples;
}