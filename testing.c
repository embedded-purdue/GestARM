#include <stdio.h>

#include "apis.h"

MpuPro processMpuRaw(MpuRaw mpuRaw);

int main() {
    MpuRaw mpuRaw;

    mpuRaw.accel[X] = 1;
    mpuRaw.accel[Y] = 2;
    mpuRaw.accel[Z] = 3;

    mpuRaw.gyro[P] = 4;
    mpuRaw.gyro[R] = 5;
    mpuRaw.gyro[W] = 6;

    printMpuRaw(mpuRaw);

    double x_double = L2D(mpuRaw.accel[X], ACCEL_MAX, ACCEL_MIN);
    printf("X double: %lf\n", x_double);
    long x_long = D2L(x_double, ACCEL_MAX, ACCEL_MIN);
    printf("X long: %ld\n", x_long);

    // example use of API call, this would return the integrated data in the return
    MpuPro mpuPro = processMpuRaw(mpuRaw);

    printMpuPro(mpuPro);
}

MpuPro processMpuRaw(MpuRaw mpuRaw) {
    MpuPro mpuPro;

    for (int i = 0; i < 3; i++) {
        mpuPro.pos[i] = mpuRaw.accel[i] * 100;
        mpuPro.gyro[i] = mpuRaw.gyro[i];
    }

    return mpuPro;
}