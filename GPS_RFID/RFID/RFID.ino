#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SS_PIN  5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);

// ตั้งค่า Wi-Fi คลังสินค้า
const char* ssid     = "June 2.4G";
const char* password = "15171974";

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();

  // เชื่อมต่อ Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected! Station Ready.");
}

void loop() {
  // ตรวจจับการแตะ RFID Tag
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  // แปลงรหัส RFID UID เป็นข้อความ String
  String tagID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    tagID += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    tagID += String(rfid.uid.uidByte[i], HEX);
  }
  tagID.toUpperCase();

  Serial.println("Scanned Tag ID: " + tagID);

  // ส่งสถานะ Departure เข้า Dashboard (ตัวอย่าง HTTP POST)
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://your-dashboard-api.com/checkout"); // ใส่ URL ของ API/Dashboard
    http.addHeader("Content-Type", "application/json");
    
    String payload = "{\"tag_id\":\"" + tagID + "\", \"status\":\"In-Transit\"}";
    int httpResponseCode = http.POST(payload);
    
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    http.end();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(3000); // หน่วงเวลาป้องกันการสแกนซ้ำติดกัน
}