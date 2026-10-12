#define TINY_GSM_MODEM_SIM7600// โมดูล A7680 ใช้คำสั่ง AT ร่วมกับ A7670/SIM7600 ได้

#include <TinyGPS++.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>

// กำหนด Pin ตามฮาร์ดแวร์ของคุณ
#define A7680_RX  26
#define A7680_TX  27
#define A7680_PWR 23

#define GPS_RX    16
#define GPS_TX    17

// ตั้งค่า APN และ MQTT Broker
const char* APN = "internet"; // TrueMove H
const char* gprsUser = "";
const char* gprsPass = "";

const char* mqtt_broker = "broker.hivemq.com"; // หรือใส่ IP/Domain ของ Broker คุณ
const int   mqtt_port   = 1883;
const char* mqtt_topic  = "tracker/BOX_001/location";
const char* tracker_id  = "BOX_001";

// ประกาศออบเจกต์ Serial และ Library
HardwareSerial SerialA7680(1);
HardwareSerial SerialGPS(2);

TinyGPSPlus gps;
TinyGsm modem(SerialA7680);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

unsigned long lastSend = 0;

void powerOnA7680() {
  pinMode(A7680_PWR, OUTPUT);
  digitalWrite(A7680_PWR, LOW);
  delay(100);
  digitalWrite(A7680_PWR, HIGH);
  delay(1000);
  digitalWrite(A7680_PWR, LOW);
  delay(3000); // รอโมดูลเริ่มทำงาน
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to MQTT Broker...");
    if (mqtt.connect(tracker_id)) {
      Serial.println(" Connected!");
    } else {
      Serial.print(" Failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // เริ่มทำงาน GPS Serial (9600 Baud Rate)
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  // เริ่มทำงาน A7680 Serial (115200 Baud Rate)
  powerOnA7680();
  SerialA7680.begin(115200, SERIAL_8N1, A7680_RX, A7680_TX);
  delay(3000);

  // เชื่อมต่อเครือข่าย Cellular
  Serial.println("Initializing modem...");
  modem.restart();

  Serial.print("Connecting to APN: ");
  Serial.println(APN);
  if (!modem.gprsConnect(APN, gprsUser, gprsPass)) {
    Serial.println(" APN Connection Failed!");
  } else {
    Serial.println(" APN Connected!");
  }

  mqtt.setServer(mqtt_broker, mqtt_port);
}

void loop() {
  // อ่านข้อมูล NMEA จาก GPS ตลอดเวลา
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }

  // เช็กการเชื่อมต่อ MQTT
  if (!mqtt.connected()) {
    connectMQTT();
  }
  mqtt.loop();

  // ส่งพิกัดทุกๆ 0.2 วินาที (200 ms)
  if (millis() - lastSend > 200) {
    lastSend = millis();

    if (gps.location.isValid()) {
      float latitude = gps.location.lat();
      float longitude = gps.location.lng();
      float speed_kmh = gps.speed.kmph();

      // จัดฟอร์แมตข้อมูลเป็น JSON
      char payload[150];
      snprintf(payload, sizeof(payload),
        "{\"tracker_id\":\"%s\",\"lat\":%.6f,\"lng\":%.6f,\"speed\":%.2f}",
        tracker_id, latitude, longitude, speed_kmh);

      mqtt.publish(mqtt_topic, payload);
      Serial.print("Data Sent: ");
      Serial.println(payload);
    } else {
      Serial.println("GPS Searching for satellites (No Fix yet)...");
    }
  }
}
