#include <stdio.h>

#include "apis.h"

double longToDouble(long val, long max, long min) {
    return (double)(val - min) / (max - min) * 2 - 1;
}

long doubleToLong(double val, long max, long min) {
    return (long)((val + 1) / 2 * (max - min) + min);
}

void printMpuRaw(MpuRaw mpuRaw) {
    printf("Accel: %ld, %ld, %ld\n", mpuRaw.accel[X], mpuRaw.accel[Y], mpuRaw.accel[Z]);
    printf("Gyro: %ld, %ld, %ld\n", mpuRaw.gyro[P], mpuRaw.gyro[R], mpuRaw.gyro[W]);
}

void printMpuPro(MpuPro mpuPro) {
    printf("Pos: %ld, %ld, %ld\n", mpuPro.pos[X], mpuPro.pos[Y], mpuPro.pos[Z]);
    printf("Gyro: %ld, %ld, %ld\n", mpuPro.gyro[P], mpuPro.gyro[R], mpuPro.gyro[W]);
}