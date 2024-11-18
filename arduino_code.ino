
#include <LiquidCrystal_I2C.h>    // Library to control the LCD display using I2C
#include <SoftwareSerial.h>       // Library to communicate with serial devices (like NodeMCU)
#include <TinyGPS++.h>            // Library to handle GPS data
#include <Wire.h>                 // Required for I2C communication
#include <SPI.h>                  // Required for RFID (if SPI communication used)
#include <MFRC522.h>               // RFID Library

 #define GREEN_LED 7              // Pin number for the Green LED
 #define RED_LED 2                // Pin number for the Red LED
 #define BUZZER 8                 // Pin number for the Buzzer

//RFID PINS 
#define SS_PIN 10  // SDA pin connected to Pin 10
#define RST_PIN 9  // RST pin connected to Pin 9

// Global variables
 String storedUID = "";    //store RFID after read it 
 bool isRFIDScanned = false;             // Flag to check if RFID is scanned
 String lastScannedRFID = "";            // Store the last scanned RFID code

 String busNo1 = "JH05DT6619";            // Bus number (fixed for this example)
 String busNo2 = "JH05DT2024";            // Bus number (fixed for this example)

 unsigned long lastPrintTime = 0;        // Store the last time GPS location was printed
 const unsigned long gpsInterval = 300000; // Time interval to update GPS data (5 minutes in milliseconds)
 bool firstPrintDone = false;            // Flag to ensure GPS prints only once initially

//RFID SETUP
MFRC522 rfid(SS_PIN, RST_PIN); // Create instance of the MFRC522 class
MFRC522::MIFARE_Key key;

// LCD setup
 LiquidCrystal_I2C lcd(0x27,20,4);     // Initialize LCD display with I2C address 0x27 (16x2)

// GPS and RFID setup
SoftwareSerial serial_connection(3,4); //tx,rx 
TinyGPSPlus gps;// GPS object to process the NMEA data

void setup()
{
  
   // Initialize LCD
   lcd.init(); 
   lcd.backlight();
   lcd.setCursor(3,0);
  
  lcd.print("GPS Init");
  delay(500);                           // Wait for 2 seconds
  lcd.clear();                           // Clear the LCD screen
  Serial.begin(9600);                //This opens up communications to the Serial monitor in the Arduino IDE
  serial_connection.begin(9600);     //This opens up communications to the GPS
  Serial.println("GPS Start");        //To show in the serial monitor that the sketch has started
  delay(1000);                           // Wait for 2 seconds
  lcd.clear();                           // Clear the LCD screen

//  NodeMCUSerial.begin(9600); 

  lcd.print("RFID Init..");
  delay(1000);                           // Wait for 2 seconds
  lcd.clear();                           // Clear the LCD screen
  // RFID INIT 
  SPI.begin();          // Initialize SPI bus
  rfid.PCD_Init();      // Initialize the RFID reader
  Serial.println("Place your RFID card near the reader...");
  lcd.print("RFID start..");
  delay(1000);                           // Wait for 2 seconds
  lcd.clear();                           // Clear the LCD screen
 

  // Set up LED and buzzer pins as OUTPUT
   pinMode(GREEN_LED, OUTPUT);   
   pinMode(RED_LED, OUTPUT);
   pinMode(BUZZER, OUTPUT);

   lcd.print("Initializing...");
   delay(1000);                           // Wait for 2 seconds
   lcd.clear();                           // Clear the LCD screen

}


void loop()
{
  while(serial_connection.available())              //While there are incoming characters  from the GPS
  {
    gps.encode(serial_connection.read());           //This feeds the serial NMEA data into the library one char at a time
  }
  if(gps.location.isUpdated())          //This will pretty much be fired all the time anyway but will at least reduce it to only after a package of NMEA data comes in
  {
    //Get the latest info from the gps object which it derived from the data sent by the GPS unit
    Serial.print("Latitude:");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude:");
    Serial.println(gps.location.lng(), 6);
    Serial.println("");
    delay(2000);
  }
    lcd.clear();   
    lcd.print("Scan Your RFID...");
    // delay(1000);
    Serial.println("RFID==");
        // Check if an RFID card is present
    if (!rfid.PICC_IsNewCardPresent()) {
      return;
    }
    
    // Read the RFID card
    if (!rfid.PICC_ReadCardSerial()) {
      return;
    }


// Print the UID of the card to the serial monitor
  Serial.print("RFID Data: ");
  storedUID = "";  // Clear the variable first
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : ""); // Prepend a 0 if necessary
    Serial.print(rfid.uid.uidByte[i], HEX);

    if (rfid.uid.uidByte[i] < 0x10) {
      storedUID += "0";  // Add leading zero if byte is less than 0x10
    }
    storedUID += String(rfid.uid.uidByte[i], HEX);  // Convert byte to hex and append to storedUID
  }

  Serial.println();  // Newline after RFID data
//  NodeMCUSerial.println("ENTRY:" + storedUID); // Send data to NodeMCU via serial communication
  Serial.print("Stored UID: ");
  Serial.println(storedUID);  // Print the stored UID
    if(storedUID){
    isRFIDScanned=true;
    }
  // Halt the card to stop reading
  rfid.PICC_HaltA();
 Serial.println("isRFIDScanned: "+ isRFIDScanned);
//  // Simulating RFID scan (replace with actual RFID logic)
  if (isRFIDScanned) { 
    Serial.println("isRFIDScanned if in: "+ storedUID);
    String currentRFID = storedUID; // Simulate an RFID reading. Replace with actual RFID reading logic

    // Check if the scanned RFID is the same as the last scanned one (indicating the user is leaving)
    if (currentRFID == lastScannedRFID) {
       Serial.println("iff currentRFID ==: "+ currentRFID +"lastScannedRFID==="+lastScannedRFID);
      handleUserExit(currentRFID); // Handle user exit
    } else {
      Serial.println("else currentRFID ==: "+ currentRFID);
      handleRFIDEntry(currentRFID); // Handle new entry
    }
  } 
   delay(2000);
}

// Handle the first-time RFID scan (user entering the bus)
void handleRFIDEntry(String rfid) {
  String studentBusNo = "JH05DT6616";
  Serial.println("handleRFIDEntry func inn: "+ rfid);
   lastScannedRFID = rfid;  // Store the RFID code for future comparison
    successBuzzerTone(); //
   lcd.clear();              // Clear the LCD
   lcd.print("RFID Scanned"); // Display "RFID Scanned" on the LCD screen
   delay(1000);
  // add bus number
  if(rfid=='01cdfa00'||rfid=='e9fbfc00'){
    studentBusNo="JH05DT6619";
  }else if(rfid=='7a211902'||rfid=='ac45df00'){
    studentBusNo="JH05DT2024";
  }
  Serial.println("STUDENT_RFID:" + rfid);
  Serial.println("STUDENT_BUSNO:" + studentBusNo);
  
  // // Simulate checking the validity of the user (this part should be replaced with actual server validation)
  bool isValidUser = true; // Simulate a valid user for demonstration (replace with actual check)

   if (isValidUser) {
      lcd.clear();
      lcd.print("Seat Booked");    // Show seat booked message
      digitalWrite(GREEN_LED, HIGH); // Turn on Green LED to indicate success
      digitalWrite(RED_LED, LOW);   // Turn off Red LED
     tone(BUZZER, 1000, 500);      // Beep the buzzer for success
   } else {
      lcd.clear();
      lcd.print("Access Denied");    // Show access denied message
      digitalWrite(RED_LED, HIGH);   // Turn on Red LED to indicate failure
      digitalWrite(GREEN_LED, LOW);  // Turn off Green LED
     tone(BUZZER, 500, 500);        // Beep buzzer for failure
   }
}

// Handle the second-time RFID scan (user leaving the bus)
void handleUserExit(String rfid) {
  Serial.println("handleUserExit ====");
//   tone(BUZZER, 1000, 500);  // Activate the buzzer with a 1000 Hz tone for 500 ms
    failureBuzzerTone(); // failur buzzer sound
    lcd.clear();              // Clear the LCD
    lcd.print("User Exit");   // Display "User Exit" on the LCD screen

  // // Send exit data (RFID code) to NodeMCU
  //  NodeMCUSerial.println("EXIT:" + rfid); // Send data to NodeMCU indicating the user is exiting the bus
}

// Send GPS data periodically (every 5 minutes)
void sendGPSData() {
  // // Prepare GPS data in the format "latitude,longitude"
   String gpsData = String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
  
  // // Send the GPS data to NodeMCU
  //  NodeMCUSerial.println("GPS:" + gpsData); 
}

//SUCCESS BUZZER TONE 
void successBuzzerTone() {
  tone(BUZZER, 1000, 100);  // Play a 1000 Hz tone for 100 ms
  delay(150);                // Pause for a brief moment
  tone(BUZZER, 1200, 100);   // Play a 1200 Hz tone for 100 ms
  delay(150);
  tone(BUZZER, 1500, 100);   // Play a 1500 Hz tone for 100 ms
  delay(150);
}

//FAIL BUZZER TONE
void failureBuzzerTone() {
  tone(BUZZER, 2000, 1500);   // Play a 2000 Hz tone for 2000 ms
}
