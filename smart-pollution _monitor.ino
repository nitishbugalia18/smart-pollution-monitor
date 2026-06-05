/*
 * IoT Smart Pollution & Safety Monitoring System
 * Team: Nitish Bugalia (23BEC1266), Sachin Kumar Payal (23BEC1033)
 * Submitted to: Sofana Reka S
 *
 * Hardware: ESP8266 (NodeMCU)
 * Sensors: MQ-135 (gas/air quality), MQ-3 (alcohol), DHT11 (temp & humidity)
 * Actuators: Fan (relay), Buzzer
 */

#include <ESP8266WiFi.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// ====== PIN DEFINITIONS ======
#define DHTPIN       D2
#define DHTTYPE      DHT11
#define MQ2_PIN      A0
#define MQ135_PWR    D6   // Power pin for MQ-135
#define MQ3_PWR      D7   // Power pin for MQ-3 (alcohol sensor)
#define RELAY_PIN    D1   // Relay for fan (Active HIGH)
#define BUZZER_PIN   D5   // Buzzer (Active HIGH)

// ====== WIFI CREDENTIALS ======
const char* ssid     = "Airtel_rata_9765";      // Replace with your WiFi name
const char* password = "Air@31283";  // Replace with your WiFi password

// ====== SERVER DETAILS ======
const char* serverIP   = "192.168.43.91";     // Replace with your Flask server IP
const int   serverPort = 5000;

// ====== THRESHOLDS ======
#define TEMP_THRESHOLD   30.0
#define MQ135_THRESHOLD  300
#define MQ3_THRESHOLD    200

DHT dht(DHTPIN, DHTTYPE);

float safeFloat(float val) {
  return isnan(val) ? 0.0 : val;
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Smart Pollution Monitor Booting ===");

  pinMode(RELAY_PIN,   OUTPUT);
  pinMode(BUZZER_PIN,  OUTPUT);
  pinMode(MQ135_PWR,   OUTPUT);
  pinMode(MQ3_PWR,     OUTPUT);

  digitalWrite(RELAY_PIN,  LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(MQ135_PWR,  LOW);
  digitalWrite(MQ3_PWR,    LOW);

  dht.begin();
  delay(2000);

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection FAILED. Running in offline mode.");
  }
}

void loop() {

  // Read DHT11
  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();

  // Read MQ-135 (Air Quality)
  digitalWrite(MQ135_PWR, HIGH);
  delay(2000);
  int mq135Value = analogRead(MQ2_PIN);
  digitalWrite(MQ135_PWR, LOW);
  delay(200);

  // Read MQ-3 (Alcohol)
  digitalWrite(MQ3_PWR, HIGH);
  delay(2000);
  int mq3Value = analogRead(MQ2_PIN);
  digitalWrite(MQ3_PWR, LOW);

  // Debug output
  Serial.println("\n==============================");
  if (isnan(temperature)) {
    Serial.println("WARNING: DHT11 read failed! Check wiring.");
  } else {
    Serial.printf("Temperature : %.2f C\n", temperature);
    Serial.printf("Humidity    : %.2f %%\n", humidity);
  }
  Serial.printf("MQ-135 (Air): %d  (threshold: %d)\n", mq135Value, MQ135_THRESHOLD);
  Serial.printf("MQ-3 (Alc.) : %d  (threshold: %d)\n", mq3Value,   MQ3_THRESHOLD);

  // Check thresholds
  bool tempAlert    = (!isnan(temperature) && temperature > TEMP_THRESHOLD);
  bool gasAlert     = (mq135Value > MQ135_THRESHOLD);
  bool alcoholAlert = (mq3Value   > MQ3_THRESHOLD);
  bool anyAlert     = tempAlert || gasAlert || alcoholAlert;

  if (anyAlert) {
    Serial.println("ALERT TRIGGERED:");
    if (tempAlert)    Serial.println("  - High temperature");
    if (gasAlert)     Serial.println("  - Poor air quality (gas/smoke)");
    if (alcoholAlert) Serial.println("  - Alcohol detected (ignition lock engaged)");
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(RELAY_PIN,  HIGH);
  } else {
    Serial.println("Environment: SAFE");
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RELAY_PIN,  LOW);
  }

  // Send data to Flask server
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    String serverURL = "http://" + String(serverIP) + ":" + String(serverPort) + "/save_data";
    http.begin(client, serverURL);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> jsonData;
    jsonData["location"]      = "Lab-AB3";
    jsonData["temperature"]   = safeFloat(temperature);
    jsonData["humidity"]      = safeFloat(humidity);
    jsonData["mq135"]         = mq135Value;
    jsonData["mq3_alcohol"]   = mq3Value;
    jsonData["temp_alert"]    = tempAlert    ? 1 : 0;
    jsonData["gas_alert"]     = gasAlert     ? 1 : 0;
    jsonData["alcohol_alert"] = alcoholAlert ? 1 : 0;
    jsonData["alert_status"]  = anyAlert     ? 1 : 0;

    String requestBody;
    serializeJson(jsonData, requestBody);

    int httpCode = http.POST(requestBody);
    Serial.printf("Data sent to server. HTTP response: %d\n", httpCode);

    http.end();
  } else {
    Serial.println("WiFi not connected — data not sent.");
  }

  Serial.println("Next reading in 10 seconds...");
  delay(10000);
}
