#include <WiFi.h>
#include <PubSubClient.h>
#include <TinyGPS++.h>
#include <ArduinoJson.h>

// --- ตั้งค่า WiFi ---
const char* ssid = "June 2.4G";
const char* password = "15171974";

// --- ตั้งค่า MQTT ---
const char* broker = "broker.emqx.io";
const char* topic_control = "myname9988/control"; // คอยรับคำสั่ง
const char* topic_pub = "myname9988/gpsdata";     // ส่งพิกัดออก
const char* mqtt_client_id = "esp32_gps_tracker";

// --- ตั้งค่า GPS ---
#define GPS_RX_PIN 33
#define GPS_TX_PIN 32
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;

WiFiClient espClient;
PubSubClient mqtt(espClient);

// --- ตัวแปรควบคุมระบบ Tracking ---
bool isTracking = false;
String activeUID = "";
unsigned long lastSendTime = 0;
const int sendInterval = 2000;

float previousLat = 0.0;
float previousLng = 0.0;
float totalDistance = 0.0;

void setup_wifi() {
  delay(10);
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

// ฟังก์ชันรับข้อความ MQTT จากตัวควบคุม (RFID)
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("Received Control Message: " + message);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (!error) {
    String action = doc["action"].as<String>();
    String uid = doc["uid"].as<String>();

    if (action == "START") {
      isTracking = true;
      activeUID = uid;
      totalDistance = 0.0;
      previousLat = 0.0;
      previousLng = 0.0;
      lastSendTime = millis() - sendInterval;
      Serial.println("[▶] Tracking Started for UID: " + activeUID);
    } 
    else if (action == "STOP") {
      isTracking = false;
      Serial.println("[■] Tracking Stopped for UID: " + activeUID);

      // ส่งแพ็กเกจสรุปจบการทำงาน
      String jsonPayload = "{\"uid\":\"" + activeUID + "\",";
      jsonPayload += "\"lat\":0,\"lng\":0,\"speed\":0,";
      jsonPayload += "\"dist\":" + String(totalDistance, 2) + ",";
      jsonPayload += "\"rssi\":" + String(WiFi.RSSI()) + ",";
      jsonPayload += "\"status\":\"completed\"}";
      mqtt.publish(topic_pub, jsonPayload.c_str());
    }
  }
}

void reconnectMQTT() {
  while (!mqtt.connected()) {
    if (mqtt.connect(mqtt_client_id)) {
      Serial.println("MQTT Connected (GPS Tracker)");
      mqtt.subscribe(topic_control); // ติดตาม Topic ควบคุม
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  setup_wifi();
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);
  Serial.println("GPS Tracker Ready!");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) setup_wifi();
  if (!mqtt.connected()) reconnectMQTT();
  mqtt.loop();

  // อ่านข้อมูลพิกัดจากโมดูล GPS
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // ส่งข้อมูลพิกัดไปยัง Broker เมื่อสถานะเป็น Tracking
  if (isTracking && (millis() - lastSendTime >= sendInterval)) {
    lastSendTime = millis();

    float lat = 0.0;
    float lng = 0.0;
    float speedKmph = 0.0;
    int rssi = WiFi.RSSI();

    if (gps.location.isValid()) {
      lat = gps.location.lat();
      lng = gps.location.lng();
      speedKmph = gps.speed.kmph();

      if (previousLat != 0.0 && previousLng != 0.0 && (lat != previousLat || lng != previousLng)) {
        totalDistance += TinyGPSPlus::distanceBetween(previousLat, previousLng, lat, lng);
      }
      previousLat = lat;
      previousLng = lng;
    }

    String jsonPayload = "{\"uid\":\"" + activeUID + "\",";
    jsonPayload += "\"lat\":" + String(lat, 6) + ",";
    jsonPayload += "\"lng\":" + String(lng, 6) + ",";
    jsonPayload += "\"speed\":" + String(speedKmph, 2) + ",";
    jsonPayload += "\"dist\":" + String(totalDistance, 2) + ",";
    jsonPayload += "\"rssi\":" + String(rssi) + ",";
    jsonPayload += "\"status\":\"tracking\"}";

    Serial.println("Sending GPS Data: " + jsonPayload);
    mqtt.publish(topic_pub, jsonPayload.c_str());
  }
}