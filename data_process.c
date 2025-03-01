#include <stdio.h>
#include "apis.h"
#include <string.h>
MpuPro processMpuRaw(MpuRaw mpuRaw);
FlexPro processFlexRaw(FlexRaw flexRaw);

void processSensorDataFromFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        MpuRaw mpuRaw;
        FlexRaw flexRaw;
        int values_read;
        if (strstr(filename, "mpu") != NULL) { 
            values_read = sscanf(line, "%ld,%ld,%ld,%ld,%ld,%ld",
                                 &mpuRaw.accel[X], &mpuRaw.accel[Y], &mpuRaw.accel[Z],
                                 &mpuRaw.gyro[P], &mpuRaw.gyro[R], &mpuRaw.gyro[W]);
            if (values_read != 6) {
                printf("Invalid MPU CSV format: %s", line);
                continue;
            }

            printMpuRaw(mpuRaw);
            MpuPro mpuPro = processMpuRaw(mpuRaw);
            printMpuPro(mpuPro);

        } else if (strstr(filename, "flex") != NULL) {
            int index;
            values_read = sscanf(line, "%d,%ld", &index, &flexRaw);

            if (values_read != 2) {
                printf("Invalid Flex CSV format: %s", line);
                continue;
            }

            printFlexRaw(flexRaw);
            FlexPro flexPro = processFlexRaw(flexRaw);
            printFlexPro(flexPro);
        }
    }

    fclose(file);
}

MpuPro processMpuRaw(MpuRaw mpuRaw) {
    MpuPro mpuPro;
    for (int i = 0; i < 3; i++) {
        mpuPro.pos[i] = mpuRaw.accel[i] * 100;
        mpuPro.gyro[i] = mpuRaw.gyro[i];
    }
    return mpuPro;
}
FlexPro processFlexRaw(FlexRaw flexRaw) {
    return flexRaw * (1000 / (FLEX_MAX - FLEX_MIN)); //just multiplying by 10 rn
}

int main() {
    processSensorDataFromFile("sample_data/mpu_accel.csv");
    processSensorDataFromFile("sample_data/flex_flex.csv");

    return 0;
}
