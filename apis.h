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

enum {
    X = 0, // Forward/Backward index
    Y = 1, // Up/Down index
    Z = 2, // Left/Right index
};

enum {
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

#endif // APIS_H