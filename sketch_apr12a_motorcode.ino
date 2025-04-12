#include <Wire.h>
#include "BluetoothSerial.h"

// ===================
// Bluetooth & LEDC Setup
// ===================

// Bluetooth Serial Instance
BluetoothSerial SerialBT;

// --- Servo PWM Configuration (using LEDC) ---
// Assign servo control pins
#define GRIPPER_SERVO_PIN   14
#define SHOULDER_SERVO_PIN  27
#define ELBOW_SERVO_PIN     26
#define WRIST1_SERVO_PIN    25
#define WRIST2_SERVO_PIN    33

// Assign LEDC channels to each servo
#define GRIPPER_CHANNEL     0
#define SHOULDER_CHANNEL    1
#define ELBOW_CHANNEL       2
#define WRIST1_CHANNEL      3
#define WRIST2_CHANNEL      4

// Servo PWM parameters
const int servoFreq = 50;           // 50 Hz for servos
const int ledcResolution = 13;      // 13-bit resolution (0..8191)
const int minDuty = (int)(0.05 * ((1 << ledcResolution) - 1));  // ~5% duty cycle
const int maxDuty = (int)(0.10 * ((1 << ledcResolution) - 1));   // ~10% duty cycle

// ===================
// Stepper Motor (Turntable/Base) Setup
// ===================
#define STEP_PIN 32      // Step pulse pin for the stepper driver
#define DIR_PIN 35       // Direction pin for the stepper driver
const float STEP_ANGLE = 1.8;         // 1.8° per step (200 steps per revolution)
unsigned long stepDelayMicros = 1000; // microseconds delay between steps
int currentBaseAngle = 90;            // Default initial position (neutral)

// ===================
// Joint Mapping & Input Setup
// ===================
#define MIN_ANGLE 0
#define MAX_ANGLE 180
#define NEUTRAL_ANGLE 90

// Gripper angle definitions:
#define GRIPPER_OPEN_ANGLE 60     
#define GRIPPER_CLOSED_ANGLE 120  

// Flick parameters
const int FLICK_OFFSET = 20;                // Fixed offset for flick movement (degrees)
const unsigned long FLICK_DURATION = 500;   // Flick duration in milliseconds (0.5 sec)

// Threshold for wrist joints (pitch/roll); these remain continuous
const float GYRO_THRESHOLD = 10.0;  // Degrees or sensor units (adjust as needed)

// Expected input count from Bluetooth is 6 values; now:
// [0]: Gripper flag (boolean: 0 or 1)
// [1]: Base flick flag (boolean)
// [2]: Shoulder flick flag (boolean)
// [3]: Elbow flick flag (boolean)
// [4]: Wrist 1 (pitch, continuous) 
// [5]: Wrist 2 (roll, continuous)
const int NUM_VALUES = 6;
float inputValues[NUM_VALUES];
String inputString = "";

// ===================
// Utility: Map a float from one range to another (for wrist joints)
// ===================
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// ===================
// Function: Set servo angle using LEDC PWM
// ===================
void setServoAngle(int channel, int angle, int pin) {
  // Map angle (0-180) to duty cycle between minDuty and maxDuty
  int duty = map(angle, 0, 180, minDuty, maxDuty);
  ledcWrite(channel, duty);
}

// ===================
// Flick Functions for Servos
// ===================
// Flick the servo by moving from neutral by a fixed offset then returning to neutral.
void flickServo(int channel, int pin) {
  // For simplicity, move in the positive direction.
  int targetAngle = NEUTRAL_ANGLE + FLICK_OFFSET;
  setServoAngle(channel, targetAngle, pin);
  delay(FLICK_DURATION);
  setServoAngle(channel, NEUTRAL_ANGLE, pin);
}

// ===================
// Function: Move the stepper motor (turntable) with a flick
// ===================
void moveTurntableFlick() {
  // For flick control, move a fixed number of steps (baseFlickSteps)
  const int baseFlickSteps = 10; // Modify as needed: number of steps to move
  // For this simple example, move in a fixed direction (e.g., clockwise)
  digitalWrite(DIR_PIN, HIGH);
  for (int i = 0; i < baseFlickSteps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelayMicros);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelayMicros);
  }
  // Optionally, you might update currentBaseAngle here;
  // however, for a flick action, we simply perform a transient movement.
  Serial.println("Turntable flick executed.");
}

// ===================
// Setup Function
// ===================
void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_Arm");

  // Initialize LEDC for each servo
  ledcSetup(GRIPPER_CHANNEL, servoFreq, ledcResolution);
  ledcAttachPin(GRIPPER_SERVO_PIN, GRIPPER_CHANNEL);
  
  ledcSetup(SHOULDER_CHANNEL, servoFreq, ledcResolution);
  ledcAttachPin(SHOULDER_SERVO_PIN, SHOULDER_CHANNEL);
  
  ledcSetup(ELBOW_CHANNEL, servoFreq, ledcResolution);
  ledcAttachPin(ELBOW_SERVO_PIN, ELBOW_CHANNEL);
  
  ledcSetup(WRIST1_CHANNEL, servoFreq, ledcResolution);
  ledcAttachPin(WRIST1_SERVO_PIN, WRIST1_CHANNEL);
  
  ledcSetup(WRIST2_CHANNEL, servoFreq, ledcResolution);
  ledcAttachPin(WRIST2_SERVO_PIN, WRIST2_CHANNEL);

  // Set initial servo positions:
  setServoAngle(GRIPPER_CHANNEL, GRIPPER_OPEN_ANGLE, GRIPPER_SERVO_PIN);
  setServoAngle(SHOULDER_CHANNEL, NEUTRAL_ANGLE, SHOULDER_SERVO_PIN);
  setServoAngle(ELBOW_CHANNEL, NEUTRAL_ANGLE, ELBOW_SERVO_PIN);
  setServoAngle(WRIST1_CHANNEL, NEUTRAL_ANGLE, WRIST1_SERVO_PIN);
  setServoAngle(WRIST2_CHANNEL, NEUTRAL_ANGLE, WRIST2_SERVO_PIN);

  // Initialize stepper motor pins
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  currentBaseAngle = NEUTRAL_ANGLE;

  Serial.println("Robot Arm Ready - Waiting for Bluetooth data (Flick control)...");
}

// ===================
// Main Loop: Receive Bluetooth Data and Execute Flick Actions
// ===================
void loop() {
  if (SerialBT.available()) {
    inputString = SerialBT.readStringUntil('\n');
    inputString.trim();
    if (inputString.length() > 0) {
      parseAndFlick(inputString);
    }
  }
}

// ===================
// Parse the incoming Bluetooth string into NUM_VALUES float values
// ===================
void parseAndFlick(String data) {
  int lastIndex = 0;
  int commaIndex = 0;
  
  for (int i = 0; i < NUM_VALUES; i++) {
    commaIndex = data.indexOf(',', lastIndex);
    String value;
    if (i < NUM_VALUES - 1) {
      value = data.substring(lastIndex, commaIndex);
    } else {
      value = data.substring(lastIndex);
    }
    inputValues[i] = value.toFloat();
    lastIndex = commaIndex + 1;
  }
  
  // --- Gripper ---
  // For gripper, value 0 = open, 1 = closed
  int gripperAngle = (inputValues[0] == 1.0) ? GRIPPER_CLOSED_ANGLE : GRIPPER_OPEN_ANGLE;
  setServoAngle(GRIPPER_CHANNEL, gripperAngle, GRIPPER_SERVO_PIN);

  // --- Accelerometer-based joints (Base, Shoulder, Elbow) as Boolean flags ---
  if (inputValues[1] == 1.0) {  // Base flick flag
    moveTurntableFlick();
  }
  if (inputValues[2] == 1.0) {  // Shoulder flick flag
    flickServo(SHOULDER_CHANNEL, SHOULDER_SERVO_PIN);
  }
  if (inputValues[3] == 1.0) {  // Elbow flick flag
    flickServo(ELBOW_CHANNEL, ELBOW_SERVO_PIN);
  }

  // --- Wrist joints: Continuous values, but using threshold logic for flick
  if (abs(inputValues[4]) > GYRO_THRESHOLD) {
    flickServo(WRIST1_CHANNEL, WRIST1_SERVO_PIN);
  }
  if (abs(inputValues[5]) > GYRO_THRESHOLD) {
    flickServo(WRIST2_CHANNEL, WRIST2_SERVO_PIN);
  }
  
  // Debug: Print parsed input values
  Serial.print("Received values: ");
  for (int i = 0; i < NUM_VALUES; i++) {
    Serial.print(inputValues[i], 1);
    Serial.print(i < NUM_VALUES - 1 ? ", " : "\n");
  }
}
