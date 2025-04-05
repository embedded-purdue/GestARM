const int flexPin = 34;              // GPIO34 → ADC1_CHANNEL_6
const float VREF = 3.3;              // ESP32 operates at 3.3V
const int ADC_RESOLUTION = 4095;     // 12-bit ADC (0–4095)
const int flexThreshold_mV = 1500;   // Flex voltage threshold (same as in original)

void setup() {
  Serial.begin(115200);
}

void loop() {
  int raw_adc = analogRead(flexPin);  // Read raw ADC value
  float voltage = (raw_adc / (float)ADC_RESOLUTION) * VREF * 1000.0; // Convert to mV

  bool is_flexed = (voltage > flexThreshold_mV); // Flex state logic

  Serial.print("Flex Sensor Raw ADC: ");
  Serial.print(raw_adc);
  Serial.print(" || Voltage: ");
  Serial.print(voltage, 2);
  Serial.println(" mV");

  Serial.print("Flexed: ");
  Serial.println(is_flexed ? "YES" : "NO");

  delay(1000); // 1 second delay
}
