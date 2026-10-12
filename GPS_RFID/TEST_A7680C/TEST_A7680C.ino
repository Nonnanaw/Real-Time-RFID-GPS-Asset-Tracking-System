// กำหนดพินที่ใช้ต่อกับโมดูล A76XX (ปรับเปลี่ยนขาให้ตรงกับวงจรจริงที่ต่ออยู่)
#define RXD2 16  // ขา RX2 ของ ESP32 -> ต่อเข้าขา TX/TXD ของโมดูล A76XX
#define TXD2 17  // ขา TX2 ของ ESP32 -> ต่อเข้าขา RX/RXD ของโมดูล A76XX

void setup() {
  Serial.begin(115200);                          // สื่อสารกับ Serial Monitor บน PC
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // สื่อสารกับโมดูล A76XX
  
  Serial.println("\n--- ESP32 Serial Bridge Ready ---");
  #define PWRKEY_PIN 23  // ขา GPIO ที่ต่อเข้ากับ PWRKEY ของโมดูล

// ปรับการเปิดเครื่องให้สมบูรณ์ขึ้น
  //pinMode(PWRKEY_PIN, OUTPUT);
  //digitalWrite(PWRKEY_PIN, LOW);   // ดึง PWRKEY ลง GND เพื่อสั่งเปิด
  //delay(1500);                     // ค้างไว้ 1.5 วินาที
  //pinMode(PWRKEY_PIN, INPUT);      // ปลดขาลอยคืนสภาพปกติ (แทนการสั่ง HIGH)
  //delay(2000);
  
  // โค้ด Serial อื่นๆ ด้านล่างตามปกติ...
}

void loop() {
  // รับค่าจาก Serial Monitor บนคอมฯ แล้วส่งไปให้ โมดูล A76XX
  while (Serial.available()) {
    Serial2.write(Serial.read());
  }
  
  // รับค่าตอบกลับจาก โมดูล A76XX แล้วส่งกลับมาแสดงบน Serial Monitor
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
}
//ทดสอบความพร้อมสื่อสาร
//พิมพ์ AT แล้วกด Enter หากขึ้น OK แสดงว่าการส่งและรับข้อมูลสมบูรณ์ 100% (หากขึ้นแค่ AT แต่ไม่มี OK ให้ลองพิมพ์ ATE1 แล้ว Enter ก่อนหนึ่งครั้ง)

//เปิดจ่ายไฟชิป GPS
//ส่งคำสั่ง AT+CGNSSPWR=1

//รอระบบโหลดเฟิร์มแวร์ (Dynamic Loading)
//รอประมาณ 8–10 วินาที (ห้ามส่งคำสั่งอื่นแทรก) จนกว่าจะมีข้อความแจ้งเตือนขึ้นว่า:
//+CGNSSPWR: READY!

//เริ่มแสดงค่าพิกัด NMEA
//เมื่อขึ้น READY! แล้ว ให้ส่งคำสั่ง AT+CGNSSTST=1 เพื่อเปิดการส่งค่าพิกัด GPS ออกมาแสดงบน Serial Monitor

//ขั้นตอนเปิดและเรียกดูพิกัด GPS

//1. สั่งเปิดไฟเลี้ยงชิป GPS:
//พิมพ์ AT+CGNSSPWR=1 แล้วกด Enter

//2. รอระบบโหลดเฟิร์มแวร์ (8–10 วินาที):
//รอจนกว่าจะมีข้อความ +CGNSSPWR: READY! เด้งขึ้นมาบน Serial Monitor

//3. เรียกดูพิกัดตำแหน่ง:
//พิมพ์ AT+CGNSSINFO แล้วกด Enter

//คำสั่ง AT ทางเลือก (หากต้องการลองสั่งผ่านคอม)

//AT+CPOWD=0

//AT+CPOWD

//AT+CPOF