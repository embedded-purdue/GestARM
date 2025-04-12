#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// Define the custom UUIDs for the BLE service and characteristic.
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Callback class to handle BLE server connection events.
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("Client connected");
  };

  void onDisconnect(BLEServer* pServer) {
    Serial.println("Client disconnected");
    // Restart advertising so that other devices can connect.
    pServer->getAdvertising()->start();
  }
};

// Callback class to handle write events to the BLE characteristic.
class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    // Retrieve the value sent from the phone (expects comma-separated floats).
    String rxValue = pCharacteristic->getValue();
    Serial.print("Received data: ");
    Serial.println(rxValue);

    // Prepare an array to store the 5 floating point values.
    float values[5];
    int count = 0;
    int startIndex = 0;
    int commaIndex = rxValue.indexOf(',');

    // Parse the string by looking for commas.
    while (commaIndex != -1 && count < 5) {
      String token = rxValue.substring(startIndex, commaIndex);
      values[count] = token.toFloat();
      count++;
      startIndex = commaIndex + 1;
      commaIndex = rxValue.indexOf(',', startIndex);
    }
    // Get the last token (or the only token if no comma is found).
    if (count < 5) {
      String token = rxValue.substring(startIndex);
      if (token.length() > 0) {
        values[count] = token.toFloat();
        count++;
      }
    }

    // Validate that we have exactly 5 values.
    if (count == 5) {
      Serial.println("Parsed float values:");
      for (int i = 0; i < 5; i++) {
         Serial.print("Value ");
         Serial.print(i);
         Serial.print(": ");
         Serial.println(values[i]);
      }
    } else {
      Serial.print("Error: Expected 5 values but received ");
      Serial.println(count);
    }
  }
};

void setup() {
  // Start the Serial Monitor.
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  // Initialize the BLE device with a user-defined name.
  BLEDevice::init("ESP32_BLE_Device");

  // Create a BLE server and set the connection callbacks.
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create a BLE service.
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE characteristic with READ and WRITE properties.
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID,
                                          BLECharacteristic::PROPERTY_READ  |
                                          BLECharacteristic::PROPERTY_WRITE
                                        );

  // Attach the write callback so that when data is received, our onWrite() method is triggered.
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // Optionally set an initial value (this will be visible to clients upon connection).
  pCharacteristic->setValue("Send 5 comma-separated floats");
  
  // Start the service.
  pService->start();

  // Start advertising the BLE service.
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);  // Helps with iPhone connectivity.
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->start();

  Serial.println("BLE Advertising has started. Waiting for a client connection...");
}

void loop() {
  // The onWrite callback handles incoming data.
  delay(2000);
}
