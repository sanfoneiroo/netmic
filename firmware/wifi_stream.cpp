#include "wifi_stream.h"
#include <WiFi.h>
#include <WiFiUdp.h>

// CONFIGURAÇÕES DE REDE
#define WIFI_SSID "Domingo.2G" 
#define WIFI_PASSWORD "domingolindo"
const IPAddress destIP(192,168,0,100); // IP do receptor
#define UDP_PORT 5004

// RTP
static uint16_t rtp_seq = 0;
static uint32_t rtp_timestamp = 0;
static const uint32_t rtp_ssrc = 0x12345678;

WiFiUDP udp;

void wifi_stream_init() {

    Serial.print("Conectando Wi-Fi...");
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("Conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void wifi_stream_send(int16_t *buffer, int samples) {
    uint8_t packet[12 + samples * sizeof(int16_t)];

    // RTP HEADER (12 bytes)
    packet[0] = 0x80;      
    packet[1] = 96;        

    packet[2] = rtp_seq >> 8;
    packet[3] = rtp_seq & 0xFF;

    packet[4] = (rtp_timestamp >> 24) & 0xFF;
    packet[5] = (rtp_timestamp >> 16) & 0xFF;
    packet[6] = (rtp_timestamp >> 8) & 0xFF;
    packet[7] = rtp_timestamp & 0xFF;

    packet[8]  = (rtp_ssrc >> 24) & 0xFF;
    packet[9]  = (rtp_ssrc >> 16) & 0xFF;
    packet[10] = (rtp_ssrc >> 8) & 0xFF;
    packet[11] = rtp_ssrc & 0xFF;

    // Copia áudio
    memcpy(packet + 12, buffer, samples * sizeof(int16_t));

    // Envia unicast
    IPAddress destIP(ipgateway);
    udp.beginPacket(destIP, UDP_PORT);
    udp.write(packet, sizeof(packet));
    udp.endPacket();

    // Atualiza RTP
    rtp_seq++;
    rtp_timestamp += samples;
}