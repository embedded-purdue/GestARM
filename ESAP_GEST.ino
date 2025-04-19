#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <BluetoothSerial.h>

// ——————————————————————————————————————————————————
// 3‑axis vector type
struct Vector3 {
  float x, y, z;
  Vector3(float x=0, float y=0, float z=0): x(x), y(y), z(z) {}
  Vector3 operator+(const Vector3& o) const { return Vector3(x+o.x, y+o.y, z+o.z); }
  Vector3 operator-(const Vector3& o) const { return Vector3(x-o.x, y-o.y, z-o.z); }
  Vector3 operator/(float c) const      { return Vector3(x/c,   y/c,   z/c);   }
  void print(const char* lbl="") const {
    Serial.print(lbl);
    Serial.print(" X:"); Serial.print(x,3);
    Serial.print(" Y:"); Serial.print(y,3);
    Serial.print(" Z:"); Serial.println(z,3);
  }
};

// ——————————————————————————————————————————————————
// Forward declarations
Vector3 readAccel();
Vector3 calcGyro(const Vector3&);
void   updateHistory(const Vector3&);
Vector3 detectFlick();
int    getSign(float);
void   calibrateAccelerometer();
void   initEspNow();

// ——————————————————————————————————————————————————
// Bluetooth + ESP‑NOW
BluetoothSerial SerialBT;
uint8_t broadcastAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
typedef struct {
  float values[6];
} struct_message;
struct_message myData;

// ——————————————————————————————————————————————————
// MPU & flick detection
Adafruit_MPU6050 mpu;
#define FLICK_THRESH    5.0f
#define HISTORY_SIZE   10
#define LOOP_DELAY     500      // ← now 500 ms

Vector3 accelOffset;
Vector3 accelHistory[HISTORY_SIZE];

// ——————————————————————————————————————————————————
// Flex sensor
const int flexPin          = 34;    // ADC1_CH6
const float VREF           = 3.3f;  // volts
const int ADC_RESOLUTION   = 4095;  // 12‑bit
const int flexThreshold_mV = 10;    // below this = flexed

// ——————————————————————————————————————————————————
void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT); digitalWrite(2, LOW);

  // MPU init & calibration
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050");
    while (1) delay(10);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  Serial.println("Calibrating MPU...");
  calibrateAccelerometer();
  Serial.println("Calibration complete\n");

  // Bluetooth
  SerialBT.begin("ESP32_Transmitter");
  Serial.println("Bluetooth started");

  // ESP‑NOW
  initEspNow();

  // initialize history buffer
  for (int i = 0; i < HISTORY_SIZE; i++) {
    accelHistory[i] = Vector3();
  }
}

void loop() {
  // 1) Read accel & compute gyro proxy
  Vector3 accel = readAccel();
  Vector3 gyro  = calcGyro(accel);

  // 2) Flick detection
  updateHistory(accel);
  Vector3 flick = detectFlick();
  if (flick.x || flick.y || flick.z) {
    flick.print("Flick ");
    digitalWrite(2, HIGH);
    delay(50);
  }

  // 3) Flex sensor
  int raw     = analogRead(flexPin);
  float mV    = raw / (float)ADC_RESOLUTION * VREF * 1000.0f;
  bool flexed = (mV < flexThreshold_mV);
  if (flexed) {
    Serial.println("FLEXED");
    digitalWrite(2, HIGH);
    delay(10);
  }
  digitalWrite(2, LOW);

  // 4) Pack into array
  myData.values[0] = flick.x;
  myData.values[1] = flick.y;
  myData.values[2] = flick.z;
  myData.values[3] = gyro.x;       // pitch
  myData.values[4] = gyro.y;       // roll
  myData.values[5] = flexed ? 1.0f : 0.0f;

  // —— Serial output in the *receiver* style ——
  Serial.print("TX → [");
  for (int i = 0; i < 6; i++) {
    Serial.print(myData.values[i], 3);
    if (i < 5) Serial.print(", ");
  }
  Serial.println("]");
  Serial.print("  Flick X: ");    Serial.println(myData.values[0], 3);
  Serial.print("  Flick Y: ");    Serial.println(myData.values[1], 3);
  Serial.print("  Flick Z: ");    Serial.println(myData.values[2], 3);
  Serial.print("  Gyro Pitch: "); Serial.println(myData.values[3], 3);
  Serial.print("  Gyro Roll: ");  Serial.println(myData.values[4], 3);
  Serial.print("  Flexed: ");     Serial.println((int)myData.values[5]);
  Serial.println("-------------------------");

  // 5) Send via ESP-NOW
  esp_err_t res = esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
  if (res == ESP_OK) {
    Serial.println("ESP-NOW send OK");
    SerialBT.println("ESP-NOW send OK");
  } else {
    Serial.print("Send err: "); Serial.println(res);
    SerialBT.print("Send err: "); SerialBT.println(res);
  }

  delay(LOOP_DELAY);
}
// ——————————————————————————————————————————————————
Vector3 readAccel() {
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);
  return Vector3(a.acceleration.x, a.acceleration.y, a.acceleration.z) - accelOffset;
}

Vector3 calcGyro(const Vector3& accel) {
  float pitch = (accel.z < -4 && fabs(accel.y) > 4) ? accel.y * 9 : 0;
  float roll  = (accel.z < -4 && fabs(accel.x) > 4) ? accel.x * 9 : 0;
  return Vector3(pitch, roll, 0);
}

void updateHistory(const Vector3& v) {
  for (int i = HISTORY_SIZE - 1; i > 0; i--) {
    accelHistory[i] = accelHistory[i - 1];
  }
  accelHistory[0] = v;
}

Vector3 detectFlick() {
  Vector3 flick(0, 0, 0);
  for (int axis = 0; axis < 3; axis++) {
    float head = axis==0 ? accelHistory[0].x 
               : axis==1 ? accelHistory[0].y 
                          : accelHistory[0].z;
    int s = getSign(head);
    if (fabs(head) > FLICK_THRESH) {
      for (int i = 1; i < HISTORY_SIZE; i++) {
        float v = axis==0 ? accelHistory[i].x 
                : axis==1 ? accelHistory[i].y 
                           : accelHistory[i].z;
        if (fabs(v) > FLICK_THRESH && getSign(v) == -s) {
          for (int j = i + 1; j < HISTORY_SIZE; j++) {
            float w = axis==0 ? accelHistory[j].x 
                    : axis==1 ? accelHistory[j].y 
                               : accelHistory[j].z;
            if (fabs(w) > FLICK_THRESH && getSign(w) == s) {
              if      (axis==0) flick.x = s;
              else if (axis==1) flick.y = s;
              else              flick.z = s;
            }
          }
        }
      }
    }
  }
  return flick;
}

int getSign(float v) {
  return (v < 0) ? -1 : 1;
}

void calibrateAccelerometer() {
  const int SAMPLES = 500;
  Vector3 sum;
  Serial.println("Keep MPU flat and still...");
  delay(500);
  for (int i = 0; i < SAMPLES; i++) {
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);
    sum = sum + Vector3(a.acceleration.x, a.acceleration.y, a.acceleration.z);
    if (i % 100 == 0) Serial.print('.');
    delay(10);
  }
  accelOffset = sum / SAMPLES;
  Serial.println("\nOffset:");
  accelOffset.print("");
}

void initEspNow() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init error");
    SerialBT.println("ESP-NOW init error");
    return;
  }
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, broadcastAddress, sizeof(broadcastAddress));
  peer.channel = 1;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Failed to add peer");
    SerialBT.println("Failed to add peer");
    return;
  }
  Serial.println("ESP-NOW ready");
  SerialBT.println("ESP-NOW ready");
}
