// === Receiver (ESP32) ===

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

// must match transmitter
typedef struct {
  float values[6];
} struct_message;
struct_message incoming;

// new callback signature for ESP32 IDF v5+
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len != sizeof(incoming)) {
    Serial.print("Bad packet size: "); Serial.println(len);
    return;
  }
  memcpy(&incoming, data, sizeof(incoming));

  // ** Print RX array for verification **
  Serial.print("RX → [");
  for (int i = 0; i < 6; i++) {
    Serial.print(incoming.values[i], 3);
    if (i < 5) Serial.print(", ");
  }
  Serial.println("]");

  // (Optional) field‑by‑field breakdown
  Serial.print("  Flick X: ");    Serial.println(incoming.values[0], 3);
  Serial.print("  Flick Y: ");    Serial.println(incoming.values[1], 3);
  Serial.print("  Flick Z: ");    Serial.println(incoming.values[2], 3);
  Serial.print("  Gyro Pitch: "); Serial.println(incoming.values[3], 3);
  Serial.print("  Gyro Roll: ");  Serial.println(incoming.values[4], 3);
  Serial.print("  Flexed: ");     Serial.println(incoming.values[5], 0);
  Serial.println("-------------------------");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);
  Serial.println("Receiver ready");
}

void loop() {
  // nothing here—packets trigger onDataRecv()
}
