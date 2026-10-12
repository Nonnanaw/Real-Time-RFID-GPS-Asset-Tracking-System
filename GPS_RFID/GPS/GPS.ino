#include <TinyGPSPlus.h>

// The TinyGPSPlus object
TinyGPSPlus gps;

// Use HardwareSerial 2 on the ESP32
HardwareSerial gpsSerial(2);

// Define pins for Serial 2
const int RXD2 = 16;
const int TXD2 = 17;

const uint32_t GPS_BAUD = 9600; // Default baud rate for NEO-8M

void setup() {
  // Start the USB serial monitor
  Serial.begin(115200);
  
  // Start the GPS hardware serial port
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  Serial.println("ESP32 GPS Serial2 Started");
}

void loop() {
  // Read data from GPS while available
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    // Encode data into TinyGPS++ object
    if (gps.encode(c)) {
      displayInfo();
    }
  }

  // Check if no data is coming from the GPS for 5 seconds
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("No GPS detected: Check wiring.");
    delay(5000);
  }
}

void displayInfo() {
  if (gps.location.isValid()) {
    Serial.print("Latitude: "); 
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude: "); 
    Serial.println(gps.location.lng(), 6);
    Serial.print("Altitude: "); 
    Serial.println(gps.altitude.meters()); // Corrected line

  } else {
    Serial.println("Location: Not Available (Waiting for Satellites Fix...)");
  }
  
  if (gps.date.isValid()) {
    Serial.print("Date: ");
    Serial.print(gps.date.month()); Serial.print("/");
    Serial.print(gps.date.day()); Serial.print("/");
    Serial.println(gps.date.year());
  }
  
  Serial.println();
  delay(5000);
}
