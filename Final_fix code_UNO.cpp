#include <LiquidCrystal_I2C.h>    // Library to control the LCD display using I2C
#include <SoftwareSerial.h>       // Library to communicate with serial devices (like NodeMCU)
#include <TinyGPS++.h>            // Library to handle GPS data
#include <Wire.h>                 // Required for I2C communication
#include <SPI.h>                  // Required for RFID (if SPI communication used)
#include <MFRC522.h>              // RFID Library

// Pin Definitions
#define GREEN_LED 6               // Pin number for Green LED
#define BLUE_LED 7                // Pin number for Blue LED
#define BUZZER 8                  // Pin number for Buzzer
#define SS_PIN 10                 // SDA pin connected to Pin 10 (RFID)
#define RST_PIN 9                 // RST pin connected to Pin 9 (RFID)

// Global Variables
MFRC522 rfid(SS_PIN, RST_PIN);    // RFID instance
LiquidCrystal_I2C lcd(0x27, 20, 4);  // LCD instance
TinyGPSPlus gps;                  // GPS instance
SoftwareSerial serial_connection(3, 4); // GPS serial connection (TX, RX)
String storedUID = "";            // Store RFID UID
String lastScannedRFID = "";      // Store last scanned RFID
String busNo = "JH05DT6619";      // Example bus number
double gps_lat, gps_lng;          // Store GPS coordinates
unsigned long lastGPSSendTime = 0; // Time tracker for GPS data sending
const unsigned long gpsSendInterval = 300000; // Interval for sending GPS data (1 second)

// Function Prototypes
void clearSerialBuffer();
void turnOnGreenLED(int delayTime = 200);
void turnOnBlueLED(int delayTime = 200);
void successBuzzerTone();
void failureBuzzerTone();
String waitForSerialData();
void handleRFIDEntry(String rfid, String isExit = "NO");
void sendGPSData();

void setup() {
  // Initialize Serial Communication
  Serial.begin(9600);
  serial_connection.begin(9600);

  // Initialize LEDs and Buzzer
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Initialize RFID and LCD
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();
  lcd.print("System Start");
  delay(1500); // Wait for system to initialize
}

void loop() {
  // Send GPS Data at 1-second intervals
  if (millis() - lastGPSSendTime >= gpsSendInterval) {
    sendGPSData();
    lastGPSSendTime = millis();
  }

  // Display message to scan RFID
  lcd.clear();
  lcd.print("Scan RFID");
  
  // Wait for a new RFID card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  // Read RFID UID
  storedUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) storedUID += "0"; // Add leading zero if needed
    storedUID += String(rfid.uid.uidByte[i], HEX);   // Append byte as HEX string
  }

  // Process RFID Entry
  if (storedUID.length() > 0) {
    String isExit = (storedUID == lastScannedRFID) ? "YES" : "NO";
    handleRFIDEntry(storedUID, isExit);
  }

  // Halt RFID to stop reading
  rfid.PICC_HaltA();
}

void handleRFIDEntry(String rfid, String isExit) {
  lastScannedRFID = rfid;  // Store the RFID for future comparisons
  lcd.clear();
  lcd.print("RFID Scanned");

  // Play buzzer tones based on entry/exit
  if (isExit == "NO") successBuzzerTone();
  else failureBuzzerTone();

  // Prepare API Request
  String studAPI = "STUD_RFID::" + rfid + ",STUD_BUSNO::" + busNo + ";";

  // Send API Request
  clearSerialBuffer();
  Serial.println(studAPI);

  // Wait for Response
  String respoNodeMCU = waitForSerialData();

  // Process Response
  if (respoNodeMCU.indexOf("STUD_RESPO::") != -1) {
    lcd.clear();

    // Extract response code between "STUD_RESPO::" and ";"
    int startIndex = respoNodeMCU.indexOf("STUD_RESPO::") + String("STUD_RESPO::").length();
    int endIndex = respoNodeMCU.indexOf(";", startIndex);

    if (startIndex != -1 && endIndex != -1) {
      String studRespoCode = respoNodeMCU.substring(startIndex, endIndex);
      int studRespoCodeInt = studRespoCode.toInt();

      // Validate and process the response code
      if (studRespoCodeInt == 200) {
        turnOnGreenLED(500);
        lcd.clear();
        lcd.print("Seat Book");
        successBuzzerTone();
        delay(1000);
      } else if (studRespoCodeInt == 201) {
        turnOnBlueLED(500);
        lcd.clear();
        lcd.print("Stud Left");
        successBuzzerTone();
        delay(1000);
      } else {
        lcd.clear();
        lcd.print("Seat Not Booked");
        failureBuzzerTone();
        delay(1000);
      }
    } else {
      lcd.clear();
      lcd.print("Parsing Error");
      failureBuzzerTone();
      delay(1000);
    }
  } else {
    lcd.clear();
    lcd.print("Invalid Response");
    failureBuzzerTone();
    delay(1000);
  }
}



void sendGPSData() {
  // Prepare GPS Data
  String gpsData = "BUS_NO::" + busNo + ",GPS_LAT::" + String(gps.location.lat(), 6) + ",GPS_LNG::" + String(gps.location.lng(), 6) + ";";
  
  // Send GPS Data
  Serial.println(gpsData);

  // Indicate GPS Update with Green & Blue LEDs
  for (int i = 0; i < 3; i++) {
    digitalWrite(BLUE_LED, HIGH);  // Turn ON the Blue LED
    delay(200);                    // Keep it ON for 200ms
    digitalWrite(BLUE_LED, LOW);   // Turn OFF the Blue LED
    delay(200);                    // Pause for 200ms before the next blink
  }
}

String waitForSerialData() {
  String response = "";
  unsigned long startTime = millis();

  while (millis() - startTime < 10000) { // Wait for 5 seconds
    while (Serial.available()) {
      char c = Serial.read();
      response += c;
      if (c == ';') { // Return response when semicolon is detected
       response.trim(); // Trim any leading/trailing whitespace
       return response; // Trim any trailing whitespace
      }
    }
  }

  return ""; // Return empty string if timeout occurs
}


void clearSerialBuffer() {
  while (Serial.available() > 0) {
    Serial.read(); // Clear the serial buffer
  }
}

void turnOnGreenLED(int delayTime) {
  digitalWrite(GREEN_LED, HIGH);
  delay(delayTime);
  digitalWrite(GREEN_LED, LOW);
}

void turnOnBlueLED(int delayTime) {
  digitalWrite(BLUE_LED, HIGH);
  delay(delayTime);
  digitalWrite(BLUE_LED, LOW);
}

void successBuzzerTone() {
  tone(BUZZER, 1000, 100);
  delay(150);
  tone(BUZZER, 1200, 100);
  delay(150);
  tone(BUZZER, 1500, 100);
  delay(150);
}

void failureBuzzerTone() {
  tone(BUZZER, 2000, 1500);
}
