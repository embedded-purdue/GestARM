#include <stdio.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DEFAULT_VREF    1100        // ESP32 default voltage reference (mV)
#define FLEX_SENSOR_ADC_CHANNEL ADC1_CHANNEL_6 // GPIO34 (ADC1 Channel 6)

void read_flex_sensor(void *pvParameter) {
    // Configure ADC
    esp_adc_cal_characteristics_t *adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t)); // initializes and allocates ADC memory
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars); // characterizes ADC

    // Configure ADC1
    adc1_config_width(ADC_WIDTH_BIT_12); // sets ADC1 bit width to 12 bits
    adc1_config_channel_atten(FLEX_SENSOR_ADC_CHANNEL, ADC_ATTEN_DB_11); // sets ADC1 attenuation to 11 dB (0-3.6V)

    // Sample ADC1 Data
    while (1) {
        uint32_t raw_adc = adc1_get_raw(FLEX_SENSOR_ADC_CHANNEL); // reads raw ADC value
        uint32_t voltage = esp_adc_cal_raw_to_voltage(raw_adc, adc_chars); // converts raw ADC value to voltage

        bool is_flexed = (voltage > 1550); // flag that moves the claw when flexed
        
        printf("Flex Sensor Raw ADC: %lu || Voltage: %lumV\n", (unsigned long)raw_adc, (unsigned long)voltage); // prints raw ADC value and voltage (mV) to console
        printf("Flexed: %s\n", is_flexed ? "YES" : "NO"); // prints whether the sensor is flexed or not

        vTaskDelay(1000 / portTICK_PERIOD_MS); // delays for 1 second
    }

}

void app_main(void)
{
    // Task to read flex sensor continuously
    xTaskCreate(read_flex_sensor, "read_flex_sensor", 2048, NULL, 5, NULL);
}





