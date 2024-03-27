#include "config.h"

#include "mpu.h"
#include "vibration_struct.h"
#include "Adafruit_INA219.h"

#include <Wire.h>
#include <BlynkSimpleEsp32.h>

#include <DHT.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <MQTTClient.h> //MQTT Library Source: https://github.com/256dpi/arduino-mqtt
#include <ArduinoJson.h> //ArduinoJson Library Source: https://github.com/bblanchon/ArduinoJson
#include "vibration_struct.h"


#define DHTPIN 2     
#define RELAYPIN 5
#define VOLTAGEPIN A0

  
DHT dht(DHTPIN, DHT22);
Adafruit_MPU6050 mpu;
Adafruit_INA219 ina219;

BlynkTimer timer;

WiFiClientSecure wifi_client = WiFiClientSecure();
// WiFiClientSecure wifi_clientcopy = WiFiClientSecure();
PubSubClient client(wifi_client);

MQTTClient mqtt_client = MQTTClient(256); //256 indicates the maximum size for packets being published and received.
// MQTTClient mqtt_clientcopy = MQTTClient(256); //256 indicates the maximum size for packets being published and received.


double temperature;
double humidity;
Vibration vibration;
int current ;
int voltage =0;
int speed = 0;


void connectToWifi(){
    //Begin WiFi in station mode
  WiFi.mode(WIFI_STA); 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi");
  //Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wifi connected");


  Serial.println("\nconnected!");
}


void connectAWS()
{
  // Configure wifi_client with the correct certificates and keys
  wifi_client.setCACert(AWS_CERT_CA);
  wifi_client.setCertificate(AWS_CERT_CRT);
  wifi_client.setPrivateKey(AWS_CERT_PRIVATE);

  //Connect to AWS IOT Broker. 8883 is the port used for MQTT
  mqtt_client.begin(AWS_IOT_ENDPOINT, 8883, wifi_client);
  // mqtt_clientcopy.begin("broker.hivemq.com", 1883, wifi_clientcopy);


  //Set action to be taken on incoming messages
  mqtt_client.onMessage(incomingMessageHandler);
  // mqtt_clientcopy.onMessage(incomingMessageHandler);


  // Serial.print("Connecting to AWS IOT");

  //Wait for connection to AWS IoT
  while (!mqtt_client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }
  
  Serial.println();

  if(!mqtt_client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }
 

  //Subscribe to a topic
  mqtt_client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");

 

   
}

void publishMessage(float temperature, float humidity, Vibration vibration, int voltage, int current, int speed)
{
  //Create a JSON document of size 200 bytes, and populate it
  //See https://arduinojson.org/
  StaticJsonDocument<400> doc;
  // doc["elapsed_time"] = millis() - t1;
  // doc["value"] = random(1000);
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  
  doc["Vibration"]["acceleration"][0] = vibration.acceleration[0];
  doc["Vibration"]["acceleration"][1] = vibration.acceleration[1];
  doc["Vibration"]["acceleration"][2] = vibration.acceleration[2];
  doc["Vibration"]["rotation"][0] = vibration.rotation[0];
  doc["Vibration"]["rotation"][1] = vibration.rotation[1];
  doc["Vibration"]["rotation"][2] = vibration.rotation[2];

  doc["timeStamp"] = time(NULL);
  doc["turbineSpeed"] = random(1000);
  doc["current"] = current;
  doc["voltage"] = voltage;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to mqtt_client

  //Publish to the topic
  mqtt_client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  mqtt_clientcopy.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);

  // Serial.println("Sent a message");
}

void incomingMessageHandler(String &topic, String &payload) {
  Serial.println("Message received!");
  Serial.println("Topic: " + topic);
  Serial.println("Payload: " + payload);
}

int readDHT(DHT &dht, double &humidity, double &temperature){
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

BLYNK_WRITE(V1)
{
  int value = param.asInt(); // Get value as integer
  Serial.print("button toggled");
  if(value == 0){
    digitalWrite(RELAYPIN, HIGH);
  }else if(value == 1){
    digitalWrite(RELAYPIN, LOW);
  }
}


void sendData(){
  Serial.println("Sending data to bllink ..........");
  Blynk.virtualWrite(V4, temperature);
  Blynk.virtualWrite(V3, humidity);
  Blynk.virtualWrite(V2, current);
  Blynk.virtualWrite(V0, voltage);

}

void setup() {
  Serial.begin(115200);
  dht.begin();
  Wire.begin();
  setupMPU(mpu);
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  ina219.setCalibration_16V_400mA();

  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, HIGH);
  delay(3000);
  connectToWifi();
  connectAWS();

  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
  timer.setInterval(2000L, sendData);
}

void loop() {

  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  readDHT(dht, humidity, temperature);
  readMPU(mpu, vibration);

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  // Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  // Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  // Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
  
  Serial.println("");

  int volt = analogRead(VOLTAGEPIN);
  double vv = map(volt,0, 4096, 0, 1650) + 89;

  vv /= 100; // divide by 100 to get the decimal values

  voltage = vv;
  Serial.print("Voltage: ");
  Serial.print(vv);
  Serial.println("V");
  
  if(voltage >= 8 || current >= 30 ){
    digitalWrite(RELAYPIN, HIGH);
    Serial.println("VAL EXCEEDED \n");
  }else if(voltage <= 10){
    // digitalWrite(RELAYPIN, LOW);
  }

  current = current_mA; // Calculate the current
  // voltage = busvoltage;

  publishMessage(temperature, humidity, vibration, voltage, current, speed);

  Blynk.run();//Run the Blynk library
  timer.run();//Run the Blynk timer


  mqtt_client.loop();
  mqtt_clientcopy.loop();

  delay(2000);
}
