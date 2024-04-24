#include "config.h"
#include "mpu.h"
#include "vibration_struct.h"

#include "ESP32_fft.h"
#include "fft_signal.h"
#include "Adafruit_INA219.h"
#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <MQTTClient.h>   //MQTT Library Source: https://github.com/256dpi/arduino-mqtt
#include <ArduinoJson.h>  //ArduinoJson Library Source: https://github.com/bblanchon/ArduinoJson

#define DHTPIN 2
#define RELAYPIN 5
#define VOLTAGEPIN A0

DHT dht(DHTPIN, DHT22);
Adafruit_MPU6050 mpu;
Adafruit_INA219 ina219;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800);
WiFiClientSecure wifi_client = WiFiClientSecure();
WiFiClient espClient;
PubSubClient client(espClient);
MQTTClient mqtt_client = MQTTClient(256);  //256 indicates the maximum size for packets being published and received.

int current;
int voltage = 0;
double temperature;
double humidity;
float vibration_frequency = 0;
Vibration vibration;

ESP_fft FFT(FFT_N, SAMPLEFREQ, FFT_REAL, FFT_FORWARD, fft_input, fft_output);


void calculateVibrationalFreq() {

  for (int k = 0; k < FFT_N; k++) {
    readMPU(mpu, vibration);
    float magnitude = sqrt(pow(vibration.acceleration[0], 2) + pow(vibration.acceleration[1], 2) + pow(vibration.acceleration[2], 2));
    fft_input[k] = magnitude;
    delay(5);
  }

  Serial.print("Acc: ");
  Serial.print(vibration.acceleration[0]);
  Serial.print(" ");
  Serial.print(vibration.acceleration[1]);
  Serial.print(" ");
  Serial.print(vibration.acceleration[2]);
  Serial.println();

  long int t1 = micros();
  // Execute transformation
  FFT.removeDC();
  FFT.hammingWindow();
  FFT.execute();
  FFT.complexToMagnitude();
  long int t2 = micros();
  //FFT.print();
  Serial.print("Time taken: ");
  Serial.print((t2 - t1) * 1.0 / 1000);
  Serial.println(" milliseconds!");

  Serial.println();
  /*Multiply the magnitude of the DC component with (1/FFT_N) to obtain the DC component*/
  Serial.printf("DC component : %f g\n", (fft_output[0]) / FFT_N);  // DC is at [0]

  Serial.printf("Fundamental Freq : %f Hz\t Mag: %f g\n", FFT.majorPeakFreq(), (FFT.majorPeak() / 10000) * 2 / FFT_N);
  for (int i = 0; i < 10; i++) {
    Serial.printf("%f:%f\n", FFT.frequency(i), fft_output[i]);
  }

  vibration_frequency = FFT.majorPeakFreq();
}


void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("esp32/sub");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void connectToWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wifi connected");
  Serial.println("\nconnected!");
}

void connectAWS() {
  wifi_client.setCACert(AWS_CERT_CA);
  wifi_client.setCertificate(AWS_CERT_CRT);
  wifi_client.setPrivateKey(AWS_CERT_PRIVATE);

  mqtt_client.begin(AWS_IOT_ENDPOINT, 8883, wifi_client);
  mqtt_client.onMessage(incomingMessageHandler);

  Serial.print("Connecting to AWS IOT");

  while (!mqtt_client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  Serial.println();

  if (!mqtt_client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  mqtt_client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.println("Message received!");
  Serial.println("Topic: ");
  Serial.println(topic);
  Serial.println("Payload: ");

  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if ((char)payload[0] == 'o' && (char)payload[1] == 'n')  //on
  {
    digitalWrite(RELAYPIN, HIGH);
    Serial.println("Relay turned ON");
  } else if ((char)payload[0] == 'o' && (char)payload[1] == 'f' && (char)payload[2] == 'f')  //off
  {
    digitalWrite(RELAYPIN, LOW);
    Serial.println("Relay turned  off");
  }
  Serial.println();
}

void publishMessage(float temperature, float humidity, Vibration vibration, int voltage, int current, int speed) {
  //Create a JSON document of size 200 bytes, and populate it
  //See https://arduinojson.org/
  StaticJsonDocument<400> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;

  doc["Vibration"]["acceleration"][0] = vibration.acceleration[0];
  doc["Vibration"]["acceleration"][1] = vibration.acceleration[1];
  doc["Vibration"]["acceleration"][2] = vibration.acceleration[2];
  doc["Vibration"]["rotation"][0] = vibration.rotation[0];
  doc["Vibration"]["rotation"][1] = vibration.rotation[1];
  doc["Vibration"]["rotation"][2] = vibration.rotation[2];

  doc["frequency"] = vibration_frequency;
  doc["timeStamp"] = timeClient.getFormattedTime();
  doc["turbineSpeed"] = random(1000);
  doc["current"] = current;
  doc["voltage"] = voltage;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  //Publish to the topic
  mqtt_client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);

  Serial.println("Sent a message");
}

void incomingMessageHandler(String &topic, String &payload) {
  Serial.println("Message received!");
  Serial.println("Topic: " + topic);
  Serial.println("Payload: " + payload);
}

int readDHT(DHT &dht, double &humidity, double &temperature) {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return -1;
  }
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%  Temperature: ");
  Serial.print(temperature);
  Serial.println(F("°C "));
  return 0;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  Wire.begin();
  setupMPU(mpu);
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, HIGH);
  delay(3000);
  connectToWifi();
  connectAWS();
  client.setServer("broker.hivemq.com", 1883);
  client.setCallback(callback);
  timeClient.begin();
}

void loop() {
  timeClient.update();
  if (!client.connected()) {
    reconnect();
  }

  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  readDHT(dht, humidity, temperature);
  // readMPU(mpu, vibration);
  calculateVibrationalFreq();

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  // Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  // Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  // Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       ");
  Serial.print(current_mA);
  Serial.println(" mA");
  Serial.print("Power:         ");
  Serial.print(power_mW);
  Serial.println(" mW");

  Serial.println("");

  int volt = analogRead(VOLTAGEPIN);
  Serial.println(volt);
  double vv = map(volt, 0, 4096, 0, 1650) + 89;

  vv /= 100;  // divide by 100 to get the decimal values

  voltage = vv;
  Serial.print("Voltage: ");
  Serial.print(vv);
  Serial.println("V");

  if (voltage >= 10) {
    digitalWrite(RELAYPIN, HIGH);
    Serial.println("VAL EXCEEDED \n");
  } else if (voltage <= 10) {
    // digitalWrite(RELAYPIN, LOW);
  }

  current = current_mA;

  publishMessage(temperature, humidity, vibration, voltage, current, 0);

  mqtt_client.loop();
  client.loop();
  delay(2000);
}