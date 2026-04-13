# ESP32 Network Microphone (RTP over UDP)

Este projeto implementa um **microfone de rede baseado em ESP32**, capaz de capturar áudio analógico, digitalizá-lo e transmiti-lo em tempo real pela rede utilizando **RTP sobre UDP**.

O sistema foi originalmente desenvolvido como base para um experimento em Audio over IP, podendo ser empregado como microfone de rede em ambientes locais ou como plataforma de testes para sistemas de áudio em tempo real.

---

# Visão geral

O dispositivo atua como um **microfone IP**, realizando:

- Captura de áudio via ADC
- Amostragem contínua do sinal analógico
- Empacotamento em RTP
- Transmissão via UDP para um destino configurado

No lado do receptor, qualquer sistema capaz de interpretar RTP pode consumir o fluxo, incluindo o gateway Python incluso neste repositório.

## Arquitetura

Microfone analógico  
→ ESP32 (ADC + RTP/UDP)  
→ Rede IP (LAN)  
→ Receptor (Gateway / software RTP / sistema externo)

---

# Características

## Áudio
- Entrada: microfone eletreto com amplificador
- Conversão: ADC interno do ESP32 (12 bits)
- Taxa de amostragem: 16 kHz
- Buffer: blocos de 256 amostras

## Rede
- Protocolo de transporte: UDP
- Protocolo de mídia: RTP
- Campos RTP implementados:
  - Sequence number
  - Timestamp
  - SSRC

## Hardware
- ESP32
- Entrada analógica no GPIO 34 (ADC1_CHANNEL_6)

---

# Funcionamento

O firmware opera em loop contínuo:

- Captura um bloco de amostras do ADC
- Monta pacote RTP
- Encapsula em UDP
- Envia para o IP configurado
- Atualiza timestamp e sequence number

---

# Configuração

## Rede Wi-Fi e destino

```cpp
#define WIFI_SSID "sua_rede"
#define WIFI_PASSWORD "sua_senha"

const IPAddress destIP(192,168,0,100);
```
## Taxa de amostragem
```cpp
#define SAMPLE_RATE 16000
```
## Buffer de áudio
```cpp
int16_t buffer[256];
```

---

# Protocolo RTP

Cada pacote transmitido contém:

- Cabeçalho RTP (12 bytes)
- Payload de áudio PCM 16-bit

```cpp
    // ------------------------------------------------
    // Montagem do cabeçalho RTP (12 bytes)
    // ------------------------------------------------

    packet[0] = 0x80;      // Versão RTP
    packet[1] = 96;        // Payload type

    // Número de sequência (identifica ordem dos pacotes)
    packet[2] = rtp_seq >> 8;
    packet[3] = rtp_seq & 0xFF;

    // Timestamp RTP (posição temporal do áudio)
    packet[4] = (rtp_timestamp >> 24) & 0xFF;
    packet[5] = (rtp_timestamp >> 16) & 0xFF;
    packet[6] = (rtp_timestamp >> 8) & 0xFF;
    packet[7] = rtp_timestamp & 0xFF;

    // SSRC (identificador da fonte RTP)
    packet[8]  = (rtp_ssrc >> 24) & 0xFF;
    packet[9]  = (rtp_ssrc >> 16) & 0xFF;
    packet[10] = (rtp_ssrc >> 8) & 0xFF;
    packet[11] = rtp_ssrc & 0xFF;
```

Campos principais:

- Sequence Number: ordenação de pacotes
- Timestamp: referência temporal do fluxo
- SSRC: identificação da origem do stream

---

# Receptor

O projeto pode ser utilizado com qualquer receptor RTP. Um gateway simples em Python pode ser usado para:

- Receber pacotes UDP
- Remover cabeçalho RTP
- Reproduzir áudio em tempo real

---

# Uso

- Microfone de rede (streaming de áudio em LAN)
- Base para experimentos com Audio over IP
- Plataforma de testes para sistemas de baixa latência
- Fonte RTP para softwares de processamento de áudio

## Limitações
- Sem compressão de áudio (PCM puro)
- Sem controle de congestionamento
- Sem retransmissão de pacotes
- Projetado para redes locais (LAN)

---

# Licença
Projeto desenvolvido por Erwin de Mattos, de uso livre para fins educacionais e experimentais.