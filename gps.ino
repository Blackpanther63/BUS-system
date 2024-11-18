#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial GPS(RXPin, TXPin);

unsigned long lastPrintTime = 0;  // Store the last time we printed the location
const unsigned long interval = 300000;  // 5 minutes = 300,000 milliseconds
bool firstPrintDone = false;  // Flag to track if the first print has been done

void setup() {
  Serial.println("GPS START==");
  Serial.begin(9600);
  GPS.begin(GPSBaud);
}

void loop() {
  while (GPS.available() > 0) {
    gps.encode(GPS.read());
  }

  // Check if GPS location is updated
  if (gps.location.isUpdated()) {
    // If this is the first print, print immediately
    if (!firstPrintDone) {
      Serial.print("Latitude=");
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude=");
      Serial.println(gps.location.lng(), 6);
      firstPrintDone = true;  // Set flag to true after first print
      lastPrintTime = millis();  // Set the first print time
    }

    // After the first print, wait for 5 minutes before printing again
    else if (millis() - lastPrintTime >= interval) {
      lastPrintTime = millis();  // Update the last print time
      Serial.print("Latitude=");
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude=");
      Serial.println(gps.location.lng(), 6);
    }
  }
}
