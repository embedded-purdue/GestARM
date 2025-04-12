#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <BluetoothSerial.h>

// Create an instance of BluetoothSerial
BluetoothSerial SerialBT;

// === ESP‑NOW Definitions ===
// Use the broadcast address so that all ESP‑NOW peers can receive the message.
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// Define a data structure that holds 6 float values.
typedef struct struct_message {
  float values[6];
} struct_message;

struct_message myData;  // Global instance to hold our generated values

// === ESP‑NOW Initialization ===
void initEspNow() {
  // Set WiFi mode to station
  WiFi.mode(WIFI_STA);
  
  // Set the WiFi channel to a fixed value (e.g., channel 1)
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    SerialBT.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register the broadcast peer.
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1; // Must match the WiFi channel above.
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add broadcast peer");
    SerialBT.println("Failed to add broadcast peer");
    return;
  }
  
  Serial.println("ESP-NOW Initialized");
  SerialBT.println("ESP-NOW Initialized");
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_Transmitter"); // Set the Bluetooth Serial device name
  Serial.println("Transmitter Setup Started");
  SerialBT.println("Transmitter Setup Started");
  
  // Initialize ESP‑NOW.
  initEspNow();

  // Seed the random number generator (using an unconnected analog pin or millis() as available)
  randomSeed(millis());
}

void loop() {
  // Generate 6 random float values (e.g., between 0.00 and 10.00)
  for (int i = 0; i < 6; i++) {
    myData.values[i] = random(0, 1000) / 100.0;
  }
  
  // Print the generated values to both Serial and SerialBT
  Serial.println("Generated random float values:");
  SerialBT.println("Generated random float values:");
  for (int i = 0; i < 6; i++) {
    Serial.print("Value ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(myData.values[i]);
    
    SerialBT.print("Value ");
    SerialBT.print(i);
    SerialBT.print(": ");
    SerialBT.println(myData.values[i]);
  }
  
  // Send the generated data via ESP‑NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("ESP‑NOW message sent successfully");
    SerialBT.println("ESP‑NOW message sent successfully");
  } else {
    Serial.print("Error sending ESP‑NOW message: ");
    Serial.println(result);
    
    SerialBT.print("Error sending ESP‑NOW message: ");
    SerialBT.println(result);
  }
  
  delay(2000);
}