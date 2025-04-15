#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <BluetoothSerial.h>

// Neal Bluetooth =======================================
// Create an instance of BluetoothSerial
BluetoothSerial SerialBT;

// === ESP‑NOW Definitions ===
// Use the broadcast address so that all ESP‑NOW peers can receive the message.
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// Define a data structure that holds 6 float values.
typedef struct struct_message {
  int flick[3];
  float gyro[2];
  int is_flexed;
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
// Neal Bluetooth =============================================

Adafruit_MPU6050 mpu;
#define FLICK_THRESH 5 // m/s^2
#define HISTORY_SIZE 10
#define LOOP_DELAY 50 // ms

const int flexPin = 34;              // GPIO34 → ADC1_CHANNEL_6
const float VREF = 3.3;              // ESP32 operates at 3.3V
const int ADC_RESOLUTION = 4095;     // 12-bit ADC (0–4095)
const int flexThreshold_mV = 10;   // Flex voltage threshold (same as in original)

struct Vector3 {
  float x;
  float y;
  float z;

  Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

  Vector3 operator+(const Vector3& other) const {
    return Vector3(x + other.x, y + other.y, z + other.z);
  }

  Vector3 operator-(const Vector3& other) const {
    return Vector3(x - other.x, y - other.y, z - other.z);
  }

  Vector3 operator/(float c) const {
    return Vector3(x / c, y / c, z / c);
  }

  void print(const char* label = "") const {
    Serial.print(label);
    Serial.print("\tX: ");
    Serial.print(x, 3);
    Serial.print("\t\tY: ");
    Serial.print(y, 3);
    Serial.print("\t\tZ: ");
    Serial.print(z, 3);
    Serial.println("");
  }
};

Vector3 accelOffset;
Vector3 accelHistory[HISTORY_SIZE]; // Stores last 10 values

void setup(void) {
  Serial.begin(115200);

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  // Set accelerometer range to ±2G
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  
  Serial.println("Calibrating MPU6050...");
  calibrateAccelerometer();
  Serial.println("Calibration complete!");
  Serial.println("");

  SerialBT.begin("ESP32_Transmitter"); // Set the Bluetooth Serial device name
  Serial.println("Transmitter Setup Started");
  SerialBT.println("Transmitter Setup Started");
  
  // Initialize ESP‑NOW.
  initEspNow();
}

void loop() {
  // MPU code =============================================================================
  Vector3 accel = readAccel();
  Vector3 gyro = calcGyro(accel);

  updateHistory(accel);

  Vector3 flick = detectFlick();
  if (flick.x != 0 || flick.y != 0 || flick.z != 0) {
    flick.print("Flick ");
    digitalWrite(2, HIGH);
    delay(50);
  }

  // Flex code ============================================================================
  int raw_adc = analogRead(flexPin);  // Read raw ADC value
  float voltage = (raw_adc / (float)ADC_RESOLUTION) * VREF * 1000.0; // Convert to mV

  bool is_flexed = (voltage < flexThreshold_mV); // Flex state logic

  // Serial.println(voltage);
  if (is_flexed) {
    Serial.println("FLEXED");
    digitalWrite(2, HIGH);
    delay(10);
  }
  digitalWrite(2, LOW);

  myData.flick[0] = flick.x;
  myData.flick[1] = flick.y;
  myData.flick[2] = flick.z;
  myData.gyro[0] = gyro.x; //pitch
  myData.gyro[1] = gyro.y; //roll
  myData.is_flexed = is_flexed;

  // Send the generated data via ESP‑NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("ESP-NOW message sent successfully");
    SerialBT.println("ESP-NOW message sent successfully");
  } else {
    Serial.print("Error sending ESP-NOW message: ");
    Serial.println(result);
    
    SerialBT.print("Error sending ESP-NOW message: ");
    SerialBT.println(result);
  }

  delay(LOOP_DELAY);
}

Vector3 readAccel() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  Vector3 rawAccel(a.acceleration.x, a.acceleration.y, a.acceleration.z);
  Vector3 calibratedAccel =  rawAccel - accelOffset;

  return calibratedAccel;
}

Vector3 calcGyro(Vector3 accel) {
  // If the Z is negative, it means the device is tilted off flat (since its calibrated to 0)
  // and only if we tilt enough do we use x or y as the degrees of rotation (~10m/s^2 down * 9 = 90 deg)
  float pitch = accel.z < -4 ? abs(accel.y) > 4 ? accel.y * 9 : 0 : 0;
  float roll = accel.z < -4 ? abs(accel.x) > 4 ? accel.x * 9 : 0 : 0;

  return Vector3(pitch, roll, 0 /*yaw is unread*/);
}

void calibrateAccelerometer() {
  const int calibrationSamples = 500;
  Vector3 sum;

  Serial.println("Place MPU6050 flat and still for calibration...");
  delay(100);

  for (int i = 0; i < calibrationSamples; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    sum = sum + Vector3(a.acceleration.x, a.acceleration.y, a.acceleration.z);
    
    if (i % 100 == 0) Serial.print(".");
    delay(10);
  }

  // Calculate average offsets
  accelOffset = sum / calibrationSamples;

  Serial.println("\nCalibration offsets:");
  accelOffset.print("");
}

void updateHistory(Vector3 newValue) {
  // Shift all elements right (discard oldest)
  for (int i = HISTORY_SIZE - 1; i > 0; i--) {
    accelHistory[i] = accelHistory[i - 1]; // Shift right
  }
  
  // Insert new value at the front (position 0)
  accelHistory[0] = newValue;
}

Vector3 detectFlick() {
  Vector3 flick = Vector3(0, 0, 0);
  int sign = 0;
  int idx = 0;
  
  idx = 0;
  sign = getSign(accelHistory[0].x);
  if (abs(accelHistory[0].x) > FLICK_THRESH) {
    idx++;
    while(idx < HISTORY_SIZE) {
      if(abs(accelHistory[idx].x) > FLICK_THRESH && getSign(accelHistory[idx].x) == sign * -1) {
        idx++;
        while(idx < HISTORY_SIZE) {
          if(abs(accelHistory[idx].x) > FLICK_THRESH && getSign(accelHistory[idx].x) == sign) {
            flick.x = sign;
          }
          idx++;
        }
      }
      idx++;
    }
  }

  idx = 0;
  sign = getSign(accelHistory[0].y);
  if (abs(accelHistory[0].y) > FLICK_THRESH) {
    idx++;
    while(idx < HISTORY_SIZE) {
      if(abs(accelHistory[idx].y) > FLICK_THRESH && getSign(accelHistory[idx].y) == sign * -1) {
        idx++;
        while(idx < HISTORY_SIZE) {
          if(abs(accelHistory[idx].y) > FLICK_THRESH && getSign(accelHistory[idx].y) == sign) {
            flick.y = sign;
          }
          idx++;
        }
      }
      idx++;
    }
  }

  idx = 0;
  sign = getSign(accelHistory[0].z);
  if (abs(accelHistory[0].z) > FLICK_THRESH) {
    idx++;
    while(idx < HISTORY_SIZE) {
      if(abs(accelHistory[idx].z) > FLICK_THRESH && getSign(accelHistory[idx].z) == sign * -1) {
        idx++;
        while(idx < HISTORY_SIZE) {
          if(abs(accelHistory[idx].z) > FLICK_THRESH && getSign(accelHistory[idx].z) == sign) {
            flick.z = sign;
          }
          idx++;
        }
      }
      idx++;
    }
  }

  return flick;
}

int getSign(float val) {
  return val < 0 ? -1 : 1;
}
