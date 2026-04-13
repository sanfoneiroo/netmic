import socket
import numpy as np
import sounddevice as sd
import threading

ENABLE_MONITOR = True  # Ativar monitoramento de pico/média do buffer (True/False)

# -----------------------------
# Configurações UDP e áudio
# -----------------------------
UDP_PORT = 5004
SAMPLE_RATE = 16000 # Hz, deve ser o mesmo do envio RTP
BLOCK_SIZE = 256 # 256 amostras = 16 ms a 16 kHz
BUFFER_BLOCKS = 16   # ~256 ms de buffer

# -----------------------------
# Buffer circular
# -----------------------------
audio_buffer = np.zeros(BLOCK_SIZE * BUFFER_BLOCKS, dtype=np.int16)
write_idx = 0
read_idx = 0
lock = threading.Lock()

# -----------------------------
# Thread de recebimento UDP
# -----------------------------
def udp_receive():
    global write_idx
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("", UDP_PORT))
    print(f"Escutando RTP UDP :{UDP_PORT}")

    while True:
        data, addr = sock.recvfrom(2048)
        if len(data) < 12:
            continue
        payload = data[12:]  # remove header RTP
        samples = np.frombuffer(payload, dtype=np.int16)
        if len(samples) != BLOCK_SIZE:
            continue
        with lock:
            end = write_idx + BLOCK_SIZE
            if end < len(audio_buffer):
                audio_buffer[write_idx:end] = samples
            else:
                part1 = len(audio_buffer) - write_idx
                audio_buffer[write_idx:] = samples[:part1]
                audio_buffer[:BLOCK_SIZE - part1] = samples[part1:]
            write_idx = (write_idx + BLOCK_SIZE) % len(audio_buffer)

# -----------------------------
# Callback de áudio
# -----------------------------
def audio_callback(outdata, frames, time, status):
    global read_idx
    with lock:
        available = (write_idx - read_idx) % len(audio_buffer)
        if available < frames:
            outdata[:] = np.zeros((frames, 1), dtype=np.int16)
            return
        end = read_idx + frames
        if end < len(audio_buffer):
            outdata[:] = audio_buffer[read_idx:end].reshape(-1, 1)
        else:
            part1 = len(audio_buffer) - read_idx
            outdata[:part1] = audio_buffer[read_idx:].reshape(-1, 1)
            outdata[part1:] = audio_buffer[:frames-part1].reshape(-1, 1)
        read_idx = (read_idx + frames) % len(audio_buffer)

# -----------------------------
# Seleção de dispositivo
# -----------------------------
print("Dispositivos de saída disponíveis:")
devices = sd.query_devices()
output_devices = [d for d in devices if d['max_output_channels'] > 0]
for i, d in enumerate(output_devices):
    print(f"{i}: {d['name']} ({d['hostapi']})")

while True:
    try:
        dev_index = int(input("Escolha o dispositivo de saída pelo número: "))
        if 0 <= dev_index < len(output_devices):
            break
        else:
            print("Número inválido, tente novamente.")
    except ValueError:
        print("Digite um número válido.")

devicenum = output_devices[dev_index]['index']
print(f"Usando dispositivo: {output_devices[dev_index]['name']}")

# -----------------------------
# Thread de rede
# -----------------------------
threading.Thread(target=udp_receive, daemon=True).start()

# -----------------------------
# Stream de áudio
# -----------------------------
stream = sd.OutputStream(
    samplerate=SAMPLE_RATE,
    channels=1,
    dtype="int16",
    blocksize=BLOCK_SIZE,
    callback=audio_callback,
    device=devicenum
)
stream.start()
print("Player iniciado. Ctrl+C para sair.")

# -----------------------------
# Função de monitoramento
# -----------------------------
def monitor_audio(interval=1.0):
    """Exibe pico e média do buffer a cada intervalo (segundos)."""
    while True:
        if ENABLE_MONITOR:
            with lock:
                buf_snapshot = audio_buffer.copy()
            peak = np.max(np.abs(buf_snapshot))
            avg = np.mean(np.abs(buf_snapshot))
            print(f"Pico: {peak} | Média: {int(avg)}")
        stop_event.wait(interval)

# -----------------------------
# Loop principal leve
# -----------------------------
stop_event = threading.Event()
threading.Thread(target=monitor_audio, daemon=True).start()

try:
    stop_event.wait()  # espera infinita sem consumir CPU
except KeyboardInterrupt:
    stop_event.set()
    stream.stop()
    stream.close()
    print("Player encerrado.")