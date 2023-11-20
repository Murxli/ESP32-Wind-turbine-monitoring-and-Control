const int analogInPin = A0; // ZMPT101B analog output connected to pin A0
const float referenceVoltage = 3.3; // ESP32 reference voltage

void setup() {
  Serial.begin(115200);
}

void loop() {
  int sensorValue = analogRead(analogInPin);

  // Convert ADC reading to voltage
  float voltage = sensorValue * (referenceVoltage / 4095.0);

  Serial.print("Voltage: ");
  Serial.print(voltage, 2); // Print voltage with two decimal places
  Serial.println("V");

  delay(1000); 
}
