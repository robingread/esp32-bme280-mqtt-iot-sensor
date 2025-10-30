#include "credentials.h"
#include <SPI.h>
#include <WiFi.h>
#include <MQTT.h>
#include <Arduino.h>
#include <Adafruit_BME280.h>

WiFiClient net;
MQTTClient client;
Adafruit_BME280 bme;

volatile bool mqttTriggerReceived = false;

// Define LED Pins
const int POWER_LED_PIN = 4;
const int WIFI_ERROR_LED_PIN = 16;
const int MQTT_ERROR_LED_PIN = 17;

void messageReceived(String &topic, String &payload)
{
  Serial.println("incoming: " + topic + " - " + payload);
  mqttTriggerReceived = true;
}

void connectWiFi()
{
  Serial.println("Connecting to WiFi...");
  Serial.printf("SSID: %s \n", WIFI_SSID);
  Serial.printf("Password: %s \n", WIFI_PASSWORD);

  WiFi.setHostname(DEVICE_ID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nWiFi connected!");
  Serial.printf("IP Address: %s \n", WiFi.localIP());
  Serial.printf("Hostname: %s \n", WiFi.getHostname());
}

void connectMQTT()
{
  Serial.println("Connecting to MQTT network...");
  Serial.printf("MQTT Host: %s \n", MQTT_HOST);
  Serial.printf("MQTT Port: %d \n", MQTT_PORT);
  Serial.printf("MQTT Username: %s \n", MQTT_USERNAME);
  Serial.printf("MQTT Password: %s \n", MQTT_PASSWORD);
  Serial.printf("MQTT data topic: %s \n", MQTT_DATA_TOPIC);
  Serial.printf("MQTT trigger topic: %s \n", MQTT_TRIGGER_TOPIC);

  client.begin(MQTT_HOST, MQTT_PORT, net);
  client.onMessage(messageReceived);
  while (!client.connect(DEVICE_ID, MQTT_USERNAME, MQTT_PASSWORD))
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to MQTT network!");

  client.subscribe(MQTT_TRIGGER_TOPIC);
}

void connectBME280()
{
  const bool status = bme.begin(BME280_I2C_ADDR);

  if (!status)
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.println("SensorID was: 0x");
    Serial.println(bme.sensorID(), 16);
    while (1)
      delay(10);
  }
  Serial.println("i2c connected!");
}

std::string createMessage(const float &temp, const float &pressure, const float &humidity, const std::string &deviceId)
{
  std::string json_payload = "{"
                             "\"device_id\": \"" +
                             deviceId + "\","
                                        "\"readings\": ["
                                        "{\"name\": \"temperature\", \"value\": " +
                             std::to_string(temp) + "},"
                                                    "{\"name\": \"humidity\", \"value\": " +
                             std::to_string(humidity) + "},"
                                                        "{\"name\": \"pressure\", \"value\": " +
                             std::to_string(pressure) + "}"
                                                        "]"
                                                        "}";
  return json_payload;
}

void publishReading()
{
  const float temp = bme.readTemperature();
  const float humidity = bme.readHumidity();
  const float pressure = bme.readPressure() / 100.0f;
  Serial.printf("bme280 temp reading: %f \n", temp);
  Serial.printf("bme280 pressure reading: %f \n", pressure);
  Serial.printf("bme280 humidity reading: %f \n", humidity);

  const std::string payload = createMessage(temp, pressure, humidity, DEVICE_ID);
  client.publish(MQTT_DATA_TOPIC, payload.c_str());
}

void setup()
{
  pinMode(POWER_LED_PIN, OUTPUT);
  pinMode(MQTT_ERROR_LED_PIN, OUTPUT);
  pinMode(WIFI_ERROR_LED_PIN, OUTPUT);

  digitalWrite(POWER_LED_PIN, HIGH);
  digitalWrite(MQTT_ERROR_LED_PIN, LOW);
  digitalWrite(WIFI_ERROR_LED_PIN, LOW);

  Serial.begin(9600);
  connectWiFi();
  connectMQTT();
  connectBME280();
}

void loop()
{
  client.loop();

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected");
    digitalWrite(WIFI_ERROR_LED_PIN, HIGH);
    connectWiFi();
    return;
  }

  digitalWrite(WIFI_ERROR_LED_PIN, LOW);

  if (!client.connected())
  {
    Serial.println("MQTT not connected");
    digitalWrite(MQTT_ERROR_LED_PIN, HIGH);
    delay(100);
    connectMQTT();
    return;
  }

  digitalWrite(MQTT_ERROR_LED_PIN, LOW);

  if (mqttTriggerReceived)
  {
    publishReading();
    mqttTriggerReceived = false;
  }

  delay(100);
}