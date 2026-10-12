#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

// --- ตั้งค่า WiFi ---
const char* ssid = "KMITL-WiFi";
const char* password = "";

// --- ตั้งค่า MQTT ---
const char* broker = "broker.emqx.io";
const char* topic_control = "myname9988/control"; // Topic สำหรับสั่งงาน
const char* mqtt_client_id = "esp32_rfid_controller";

// --- ตั้งค่า RFID ---
#define RST_PIN 22
#define SS_PIN 5
MFRC522 rfid(SS_PIN, RST_PIN);

WiFiClient espClient;
PubSubClient mqtt(espClient);

bool isTracking = false;

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

void forceResetRFID() {
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW);
  delay(50);
  digitalWrite(RST_PIN, HIGH);
  delay(50);
  rfid.PCD_Init();
}

void reconnectMQTT() {
  while (!mqtt.connected()) {
    if (mqtt.connect(mqtt_client_id)) {
      Serial.println("MQTT Connected (RFID Controller)");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  forceResetRFID();
  setup_wifi();
  mqtt.setServer(broker, 1883);
  Serial.println("RFID Controller Ready!");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) setup_wifi();
  if (!mqtt.connected()) reconnectMQTT();
  mqtt.loop();

  // ตรวจสอบการทาบบัตร RFID
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String uidString = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      uidString += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
      uidString += String(rfid.uid.uidByte[i], HEX);
    }
    uidString.toUpperCase();

    isTracking = !isTracking;

    String jsonPayload = "{";
    jsonPayload += "\"uid\":\"" + uidString + "\",";
    if (isTracking) {
      jsonPayload += "\"action\":\"START\"";
      Serial.println("\n[▶] Command Sent: START TRACKING - UID: " + uidString);
    } else {
      jsonPayload += "\"action\":\"STOP\"";
      Serial.println("\n[■] Command Sent: STOP TRACKING - UID: " + uidString);
    }
    jsonPayload += "}";

    mqtt.publish(topic_control, jsonPayload.c_str());

    delay(2000);
    forceResetRFID();
  }
}