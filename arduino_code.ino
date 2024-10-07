#include <DHT.h>               // Library for DHT22
#include <Wire.h>              // Library for I2C (BH1750)
#include <BH1750.h>            // Library for BH1750 Light Sensor
#include <OneWire.h>           // Library for DS18B20
#include <DallasTemperature.h> // Library for DS18B20
#include <ESP8266WiFi.h>       // Library for ESP8266 WiFi
//#include <WiFi.h>            // Use this for ESP32

// Wi-Fi credentials
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

// ThingSpeak API details
const char* server = "api.thingspeak.com";
String apiKey = "Your_API_Key";

// Define sensor pins
#define DHTPIN 2                // Pin for DHT22
#define DHTTYPE DHT22           // Type for DHT22
#define SOIL_MOISTURE_PIN A0    // Pin for soil moisture
#define LDR_PIN A1              // Pin for LDR (simple light measurement)
#define DS18B20_PIN 3           // Pin for DS18B20 (soil temperature)

// Initialize sensors
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

WiFiClient client; // Initialize Wi-Fi client

void setup() {
  // Start serial communication
  Serial.begin(9600);
  
  // Start DHT22
  dht.begin();
  
  // Start BH1750 (solar radiation)
  Wire.begin();
  lightMeter.begin();
  
  // Start DS18B20 (soil temperature)
  sensors.begin();
  
  // Start Wi-Fi connection
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  // Wait until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to Wi-Fi!");
}

void loop() {
  // Reading air temperature and humidity (DHT22)
  float temperature = dht.readTemperature();    // Temperature in °C
  float humidity = dht.readHumidity();          // Relative humidity

  // Reading soil moisture
  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN); // Analog reading
  
  // Reading solar radiation (BH1750)
  float lux = lightMeter.readLightLevel();

  // Simple light reading using LDR
  int ldrValue = analogRead(LDR_PIN);

  // Reading soil temperature (DS18B20)
  sensors.requestTemperatures();               // Request temperature from DS18B20
  float soilTemperature = sensors.getTempCByIndex(0);

  // Send data to ThingSpeak
  if (client.connect(server, 80)) {  // Connect to ThingSpeak
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(temperature);
    postStr += "&field2=";
    postStr += String(humidity);
    postStr += "&field3=";
    postStr += String(soilMoistureValue);
    postStr += "&field4=";
    postStr += String(lux);
    postStr += "&field5=";
    postStr += String(ldrValue);
    postStr += "&field6=";
    postStr += String(soilTemperature);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: ");
    client.print(server);
    client.print("\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.println("Data sent to ThingSpeak");
    client.stop();
  } else {
    Serial.println("Failed to connect to ThingSpeak");
  }

  // Wait 15 seconds before the next reading (ThingSpeak requirement)
  delay(15000);
}
