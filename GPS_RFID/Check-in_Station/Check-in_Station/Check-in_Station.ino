#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <PubSubClient.h>

// กำหนดพิน RC522
#define SS_PIN  5
#define RST_PIN 22

// ตั้งค่า Wi-Fi และ MQTT
const char* ssid        = "June 2.4G";     // ใส่ชื่อ Wi-Fi
const char* password    = "15171974"; // ใส่รหัสผ่าน Wi-Fi
const char* mqtt_broker = "broker.hivemq.com";
const int   mqtt_port   = 1883;
const char* mqtt_topic  = "tracker/checkin";

// กำหนด Tracker ID ที่ต้องการจับคู่กับสถานีนี้
const char* assigned_tracker = "BOX_001";

MFRC522 rfid(SS_PIN, RST_PIN);
WiFiClient espClient;
PubSubClient mqtt(espClient);

void setupWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "RFID_Station_" + String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.println(" Connected!");
    } else {
      Serial.print(" Failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // เริ่มการทำงาน SPI และ MFRC522
  SPI.begin();
  rfid.PCD_Init();
  
  setupWiFi();
  mqtt.setServer(mqtt_broker, mqtt_port);
  
  Serial.println("RFID Check-in Station Ready! Scan a tag...");
}

void loop() {
  if (!mqtt.connected()) {
    connectMQTT();
  }
  mqtt.loop();

  // ตรวจสอบว่ามีการแตะบัตร/แท็กหรือไม่
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // แปลง UID บัตรเป็น String
  String rfid_uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) rfid_uid += "0";
    rfid_uid += String(rfid.uid.uidByte[i], HEX);
  }
  rfid_uid.toUpperCase();

  Serial.print("Scanned RFID Tag UID: ");
  Serial.println(rfid_uid);

  // สร้าง JSON Payload
  char payload[150];
  snprintf(payload, sizeof(payload),
    "{\"rfid_tag\":\"%s\",\"tracker_id\":\"%s\",\"status\":\"CHECK_IN\"}",
    rfid_uid.c_str(), assigned_tracker);

  // ส่งข้อมูลเข้า MQTT Broker
  if (mqtt.publish(mqtt_topic, payload)) {
    Serial.print("Published to MQTT: ");
    Serial.println(payload);
  } else {
    Serial.println("Failed to publish to MQTT");
  }

  // หน่วงเวลา 2 วินาที ป้องกันการอ่านซ้ำซ้อน
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(2000);
}