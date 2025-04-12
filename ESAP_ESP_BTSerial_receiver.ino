#include <WiFi.h>
#include <esp_now.h>
#include <BluetoothSerial.h>

// Create an instance of BluetoothSerial for debug output.
BluetoothSerial SerialBT;

// Define a data structure identical to the transmitter’s (with 6 float values).
typedef struct struct_message {
  float values[6];
} struct_message;

struct_message receivedData;

// ESP‑NOW receive callback (updated for the new signature)
void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2],
           recv_info->src_addr[3], recv_info->src_addr[4], recv_info->src_addr[5]);
  Serial.print("Received ESP‑NOW message from: ");
  Serial.println(macStr);
  SerialBT.print("Received ESP‑NOW message from: ");
  SerialBT.println(macStr);
  
  // Copy the incoming data into our structure.
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  
  // Print the received float values.
  Serial.println("Received float values:");
  SerialBT.println("Received float values:");
  for (int i = 0; i < 6; i++) {
    Serial.print("Value ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(receivedData.values[i]);
    
    SerialBT.print("Value ");
    SerialBT.print(i);
    SerialBT.print(": ");
    SerialBT.println(receivedData.values[i]);
  }
  Serial.println("---------------------------");
  SerialBT.println("---------------------------");
}

void initESPNowReceiver() {
  // Set device to WiFi Station mode
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP‑NOW on Receiver");
    SerialBT.println("Error initializing ESP‑NOW on Receiver");
    return;
  }
  
  // Register the receive callback
  esp_err_t err = esp_now_register_recv_cb(onDataRecv);
  if (err == ESP_OK) {
    Serial.println("ESP‑NOW Receiver Initialized");
    SerialBT.println("ESP‑NOW Receiver Initialized");
  } else {
    Serial.print("Error registering ESP‑NOW receive callback: ");
    Serial.println(err);
    SerialBT.print("Error registering ESP‑NOW receive callback: ");
    SerialBT.println(err);
  }
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_Receiver"); // Set the Bluetooth Serial device name for the receiver
  Serial.println("Receiver Setup Started");
  SerialBT.println("Receiver Setup Started");
  
  // Initialize ESP‑NOW on the receiver.
  initESPNowReceiver();
}

void loop() {
  // Data reception is handled via the ESP‑NOW callback.
  delay(2000);
}