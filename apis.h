#ifndef APIS_H
#define APIS_H

/******************************************************************************
 * General functions and definitions
 */

#define L2D(val, max, min) (longToDouble(val, max, min))
#define D2L(val, max, min) (doubleToLong(val, max, min))

// Convert long in range [min, max] to double in range [-1, 1]
double longToDouble(long val, long max, long min);
// Convert double in range [-1, 1] to long in range [min, max]
long doubleToLong(double val, long max, long min);


/******************************************************************************
 * MPU sensor APIs, enums, and definitions
 * NOTE: we will probably move this to their respective files later on
 */

#define ACCEL_MAX 100 // <ADD UNITS>
#define ACCEL_MIN -100 // <ADD UNITS>

#define POS_MAX 1000 // <ADD UNITS>
#define POS_MIN -1000 // <ADD UNITS>

#define GYRO_MAX 180 // <ADD UNITS>
#define GYRO_MIN -180 // <ADD UNITS>

enum Axes {
    X = 0, // Forward/Backward index
    Y = 1, // Up/Down index
    Z = 2, // Left/Right index
};

enum Rotations {
    P = 0, // Pitch index (rotation around Z)
    R = 1, // Roll index (rotation around X)
    W = 2, // Yaw index (rotation around Y)
};

// Raw data API from the MPU sensor
typedef struct {
    long accel[3]; // X, Y, Z in <ADD UNITS>
    long gyro[3]; // P, R, W in <ADD UNITS>
} MpuRaw;

// Processed data API from the MPU sensor
typedef struct {
    long pos[3]; // X, Y, Z in <ADD UNITS
    long gyro[3]; // P, R, W in <ADD UNITS>
} MpuPro;

void printMpuRaw(MpuRaw mpuRaw);
void printMpuPro(MpuPro mpuPro);

/******************************************************************************
 * Flex sensor APIs, enums, and definitions
 */

#define FLEX_MAX 100 // <ADD UNITS>
#define FLEX_MIN 0 // <ADD UNITS>

// Raw data API from the flex sensor
typedef long FlexRaw;

// Processed data API from the flex sensor
typedef long FlexPro;

void printFlexRaw(FlexRaw flex);
void printFlexPro(FlexPro flex);

/******************************************************************************
 * Bluetooth APIs, enums, and definitions
 */

typedef struct {
    MpuRaw mpuRaw; // Raw data from the MPU sensor
    FlexRaw flexRaw; // Raw data from the flex sensor
} SensorData;

void printSensorData(SensorData sensorData);

/******************************************************************************
 * Motor APIs, enums, and definitions
 * 
 * NOTE: The shoulder has 2 motors because it holds the most weight, 
 * but they are controlled as one motor.
 */

#define MOTOR_MAX 100 // <ADD UNITS>
#define MOTOR_MIN -100 // <ADD UNITS>

enum Motors {
    STEPPER = 0,  // Stepper motor index
    SHOULDER = 1, // Shoulder motor index
    ELBOW = 2,    // Elbow motor index
    WRIST_1 = 3,  // Wrist motor 1 index
    WRIST_2 = 4,  // Wrist motor 2 index
    GRIPPER = 5,  // Gripper motor index
};

// Command to be sent to the motor controller
typedef long MotorCommand;

// Motor control API to be sent to the motor controllers
typedef struct {
    MotorCommand motorCommands[6];
} MotorControl;

void printMotorCommand(MotorCommand motorCommand);
void printMotorControl(MotorControl motorControl);

#endif // APIS_H