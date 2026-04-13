#include "audio_capture.h"
#include "wifi_stream.h"

int16_t buffer[256];
int valordobuffer = 256;

void setup()
{
 Serial.begin(115200);
 audio_capture_init();
 wifi_stream_init();
}

void loop()
{
 audio_capture_read(buffer,valordobuffer);
 wifi_stream_send(buffer,valordobuffer);
}