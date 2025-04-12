#include <Wire.h>
#include "BluetoothSerial.h"

// ===================
// Bluetooth & LEDC Setup
// ===================

// Bluetooth Serial Instance
BluetoothSerial SerialBT;

// --- Servo PWM Setup (Using LEDC) ---
// Define servo pins on ESP32
#define GRIPPER_SERVO_PIN   14
#define SHOULDER_SERVO_PIN  27
#define ELBOW_SERVO_PIN     26
#define WRIST1_SERVO_PIN    25
#define WRIST2_SERVO_PIN    33

// Define corresponding LEDC channels for each servo
#define GRIPPER_CHANNEL     0
#define SHOULDER_CHANNEL    1
#define ELBOW_CHANNEL       2
#define WRIST1_CHANNEL      3
#define WRIST2_CHANNEL      4

// Servo PWM parameters
const int servoFreq = 50;            // 50 Hz for servos
const int ledcResolution = 13;       // 13-bit resolution (range 0..8191)
const int minDuty = (int)(0.05 * ((1 << ledcResolution) - 1)); // ~5% duty (~409 for 13-bit)
const int maxDuty = (int)(0.10 * ((1 << ledcResolution) - 1)); // ~10% duty (~819 for 13-bit)

// ===================
// Stepper Motor (Turntable/Base) Setup
// ===================
#define STEP_PIN 32   // Pin for stepping pulse
#define DIR_PIN 35    // Pin for direction
const float STEP_ANGLE = 1.8;         // 1.8° per step (200 steps/rev)
unsigned long stepDelayMicros = 1000; // Delay in microseconds between steps
int currentBaseAngle = 90;            // Start at a default mid-position (90°)

// ===================
// Mapping & Communication Setup
// ===================
#define MIN_ANGLE 0
#define MAX_ANGLE 180

// Gripper angle definitions (in degrees)
#define GRIPPER_OPEN_ANGLE 60
#define GRIPPER_CLOSED_ANGLE 120

// Expected number of comma-separated values from Bluetooth
const int NUM_VALUES = 6;
float inputValues[NUM_VALUES];  // Array to hold the incoming sensor values
String inputString = "";        // Buffer for the incoming Bluetooth string

// ===================
// Utility: Map a float from one range to another
// ===================
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// ===================
// Function: Set servo angle on a given LEDC channel
// ===================
void setServoAngle(int ledcChannel, int angle) {
  // Map angle (0-180) to duty cycle (minDuty - maxDuty)
  int duty = map(angle, 0, 180, minDuty, maxDuty);
  // Write duty cycle to LEDC channel
  ledcWrite(ledcChannel, duty);
}

// ===================
// Initialization in setup()
// ===================
void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_Arm");

  // Initialize LEDC for each servo channel
  // For each servo, attach the designated pin to a LEDC channel
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

  // Initialize servo positions to a safe default (e.g., 90° = neutral)
  setServoAngle(GRIPPER_CHANNEL, GRIPPER_OPEN_ANGLE);
  setServoAngle(SHOULDER_CHANNEL, 90);
  setServoAngle(ELBOW_CHANNEL, 90);
  setServoAngle(WRIST1_CHANNEL, 90);
  setServoAngle(WRIST2_CHANNEL, 90);

  // Setup stepper motor pins for the turntable (base)
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  currentBaseAngle = 90;  // Default center position

  Serial.println("Robot Arm Ready - Waiting for Bluetooth data (6 values)...");
}

// ===================
// Main loop: Receive data via Bluetooth and update motors
// ===================
void loop() {
  if (SerialBT.available()) {
    // Read Bluetooth data until newline (example input: "1,-10,15,0,-45,20")
    inputString = SerialBT.readStringUntil('\n');
    inputString.trim();
    if (inputString.length() > 0) {
      parseAndMap(inputString);
    }
  }
}

// ===================
// Parse the input string into 6 float values and map them to joint angles
// ===================
void parseAndMap(String data) {
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

  // --- Mapping --- 
  // 1. Gripper: Use a boolean flag – if 1 then closed, else open.
  int gripperAngle = (inputValues[0] == 1.0) ? GRIPPER_CLOSED_ANGLE : GRIPPER_OPEN_ANGLE;

  // 2. Base: Accelerometer value in [-20,20] mapped to [0,180] for stepper motor target
  int baseTargetAngle = constrain(mapFloat(inputValues[1], -20.0, 20.0, MIN_ANGLE, MAX_ANGLE), MIN_ANGLE, MAX_ANGLE);

  // 3. Shoulder, Elbow: Accelerometer values in [-20,20] mapped to [0,180]
  int shoulderAngle = constrain(mapFloat(inputValues[2], -20.0, 20.0, MIN_ANGLE, MAX_ANGLE), MIN_ANGLE, MAX_ANGLE);
  int elbowAngle    = constrain(mapFloat(inputValues[3], -20.0, 20.0, MIN_ANGLE, MAX_ANGLE), MIN_ANGLE, MAX_ANGLE);

  // 4. Wrist joints: Pitch and Roll data in [-90,90] mapped to [0,180]
  int wrist1Angle   = constrain(mapFloat(inputValues[4], -90.0, 90.0, MIN_ANGLE, MAX_ANGLE), MIN_ANGLE, MAX_ANGLE);
  int wrist2Angle   = constrain(mapFloat(inputValues[5], -90.0, 90.0, MIN_ANGLE, MAX_ANGLE), MIN_ANGLE, MAX_ANGLE);

  // ===================
  // Send commands to the servos
  // ===================
  setServoAngle(GRIPPER_CHANNEL, gripperAngle);
  setServoAngle(SHOULDER_CHANNEL, shoulderAngle);
  setServoAngle(ELBOW_CHANNEL, elbowAngle);
  setServoAngle(WRIST1_CHANNEL, wrist1Angle);
  setServoAngle(WRIST2_CHANNEL, wrist2Angle);

  // ===================
  // Move the stepper motor for the base (turntable)
  // ===================
  moveTurntable(baseTargetAngle);

  // Debug output:
  Serial.print("Gripper: "); Serial.print(gripperAngle);
  Serial.print(" | Base Target: "); Serial.print(baseTargetAngle);
  Serial.print(" | Shoulder: "); Serial.print(shoulderAngle);
  Serial.print(" | Elbow: "); Serial.print(elbowAngle);
  Serial.print(" | Wrist1: "); Serial.print(wrist1Angle);
  Serial.print(" | Wrist2: "); Serial.println(wrist2Angle);
}

// ===================
// Function: Move the stepper motor (base) towards the target angle
// ===================
void moveTurntable(int targetAngle) {
  int angleDiff = targetAngle - currentBaseAngle;
  
  if (angleDiff != 0) {
    // Determine direction: HIGH for clockwise, LOW for counterclockwise
    bool direction = (angleDiff > 0);
    digitalWrite(DIR_PIN, direction ? HIGH : LOW);
    
    // Calculate number of steps (each step is 1.8°)
    int steps = abs(angleDiff) / STEP_ANGLE;  // simple integer division
  
    // Step the motor
    for (int i = 0; i < steps; i++) {
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(stepDelayMicros);
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(stepDelayMicros);
    }
    
    // Update current base angle
    currentBaseAngle = targetAngle;
    Serial.print("Stepper moved to angle: "); Serial.println(currentBaseAngle);
  }
}
