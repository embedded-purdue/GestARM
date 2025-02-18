#include <stdio.h>

#include "apis.h"

double longToDouble(long val, long max, long min) {
    return (double)(val - min) / (max - min) * 2 - 1;
}

long doubleToLong(double val, long max, long min) {
    return (long)((val + 1) / 2 * (max - min) + min);
}

void printMpuRaw(MpuRaw mpuRaw) {
    printf("MpuRaw: {\n");
    printf("Accel: %ld, %ld, %ld\n", mpuRaw.accel[X], mpuRaw.accel[Y], mpuRaw.accel[Z]);
    printf("Gyro: %ld, %ld, %ld\n", mpuRaw.gyro[P], mpuRaw.gyro[R], mpuRaw.gyro[W]);
    printf("}\n");
}

void printMpuPro(MpuPro mpuPro) {
    printf("MpuPro: {\n");
    printf("Pos: %ld, %ld, %ld\n", mpuPro.pos[X], mpuPro.pos[Y], mpuPro.pos[Z]);
    printf("Gyro: %ld, %ld, %ld\n", mpuPro.gyro[P], mpuPro.gyro[R], mpuPro.gyro[W]);
    printf("}\n");
}

void printFlexRaw(FlexRaw flexRaw) {
    printf("FlexRaw: %ld\n", flexRaw);
}

void printFlexPro(FlexPro flexPro) {
    printf("FlexPro: %ld\n", flexPro);
}

void printMotorCommand(MotorCommand motorCommand) {   
    printf("MotorCommand: %ld\n", motorCommand);
}

void printSensorData(SensorData sensorData) {
    printf("SensorData: {\n");
    printMpuRaw(sensorData.mpuRaw);
    printFlexRaw(sensorData.flexRaw);
    printf("}\n");
}

void printMotorControl(MotorControl motorControl) {
    printf("MotorControl: {\n");
    for (int i = 0; i < 6; i++) {
        switch (i) {
            case STEPPER:   printf("Stepper:  ");  break;
            case SHOULDER:  printf("Shoulder: ");  break;
            case ELBOW:     printf("Elbow:    ");  break;
            case WRIST_1:   printf("Wrist_1:  ");  break;
            case WRIST_2:   printf("Wrist_2:  ");  break;
            case GRIPPER:   printf("Gripper:  ");  break;
        }
        printMotorCommand(motorControl.motorCommands[i]);
    }
    printf("}\n");
}