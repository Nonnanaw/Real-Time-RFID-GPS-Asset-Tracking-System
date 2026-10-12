#include <HardwareSerial.h>

#define PWRKEY_PIN 23
#define A7680_RX   16  // เปลี่ยนเป็น GPIO 16 (RX2) ต่อเข้าขา UTX ของ A7680C
#define A7680_TX   17  // เปลี่ยนเป็น GPIO 17 (TX2) ต่อเข้าขา URX ของ A7680C

HardwareSerial modemSerial(1);

float latitude = 0.0;
float longitude = 0.0;

String sendAT(String command, int timeout = 1000) {
  String response = "";
  modemSerial.println(command);
  long start = millis();
  while (millis() - start < timeout) {
    while (modemSerial.available()) {
      response += (char)modemSerial.read();
    }
  }
  return response;
}

// ฟังก์ชันแกะพิกัดจากข้อความ +CGNSSINFO
bool parseGPS(String rawData) {
  int index = rawData.indexOf("+CGNSSINFO:");
  if (index == -1) return false;

  String data = rawData.substring(index + 12);
  
  // นับจำนวนลูกน้ำ เช็กว่ามีพิกัดส่งมาหรือยัง
  int commaCount = 0;
  for (int i = 0; i < data.length(); i++) {
    if (data[i] == ',') commaCount++;
  }
  
  // ถ้าพิกัดยังไม่จับสัญญาณ จะมีแต่ลูกน้ำว่างๆ
  if (data.startsWith(",,,,,") || commaCount < 6) return false;

  // ตัดแบ่งข้อความตามลูกน้ำ
  int p[10];
  int count = 0;
  for (int i = 0; i < data.length() && count < 10; i++) {
    if (data[i] == ',') {
      p[count++] = i;
    }
  }

  String latStr = data.substring(p[3] + 1, p[4]);
  String lonStr = data.substring(p[5] + 1, p[6]);

  if (latStr.length() > 0 && lonStr.length() > 0) {
    latitude = latStr.toFloat();
    longitude = lonStr.toFloat();
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PWRKEY_PIN, OUTPUT);
  digitalWrite(PWRKEY_PIN, LOW);
  delay(1000);
  digitalWrite(PWRKEY_PIN, HIGH);
  delay(2000);

  modemSerial.begin(115200, SERIAL_8N1, A7680_RX, A7680_TX);
  Serial.println("\n--- A7680C Tracker System Ready ---");

  sendAT("AT+CGNSSPWR=1", 2000);
  sendAT("AT+CGNSSAGPS=1", 1000); // สั่งเปิดระบบ AGPS ดึงพิกัดผ่านเน็ต 4G
}

void loop() {
  String rawData = sendAT("AT+CGNSSINFO", 1000);

  if (rawData.indexOf("ERROR") != -1) {
    Serial.println("GNSS Offline! Re-enabling...");
    sendAT("AT+CGNSSPWR=1", 1000);
  } else if (parseGPS(rawData)) {
    Serial.print("GPS Fixed! Lat: ");
    Serial.print(latitude, 6);
    Serial.print(" | Lng: ");
    Serial.println(longitude, 6);
  } else {
    Serial.println("Waiting for Satellite Signal (Cold Start)...");
  }

  delay(3000);
}