#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN); // Create instance of the MFRC522 class
MFRC522::MIFARE_Key key;

void setup() {
  Serial.begin(9600);   // Initialize serial communication
  SPI.begin();          // Initialize SPI bus
  rfid.PCD_Init();      // Initialize the RFID reader
  Serial.println("Place your RFID card near the reader...");
}

void loop() {
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
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : ""); // Prepend a 0 if necessary
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();  // Newline after RFID data

  // Halt the card to stop reading
  rfid.PICC_HaltA();
}
