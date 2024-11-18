#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// WiFi credentials
const char* ssid = "POCO M2 Pro";
const char* password = "1223334444";

//const char* ssid = "the_suryadev";
//const char* password = "12345678901";


// Server endpoints
const String checkRFIDURL = "https://bus-arka-server.vercel.app/api/rfid/check";
const String updateLocationURL = "https://bus-arka-server.vercel.app/api/bus_location/update";

// WiFiClient object for HTTP requests
WiFiClient client;  // Create a WiFiClient object

// Set up Serial for communication with Arduino
void setup() {
  Serial.begin(9600);          // Communication with Arduino
  Serial.println("NodeMCU is ready to receive data from Arduino");
  Serial.setTimeout(2000);     // Set a timeout for serial reading

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void loop() {
  
  Serial.println("Loopp=====");  
  // Check if data is available from Arduino
  if (Serial.available()) {
    String receivedMessage = Serial.readStringUntil('\n'); // Read incoming string
    Serial.println("Message received from Arduino: " + receivedMessage);

         Serial.println("inn ifff=====");  
        String receivedData = Serial.readStringUntil('\n');  // Read the incoming string from Arduino
    
        // Parse received data (assuming format "COMMAND:DATA")
        if (receivedData.startsWith("ENTRY:")) {
          Serial.println("inn ifff startsWith=====");  
          String rfidCode = receivedData.substring(6);
          sendRFIDCheck(rfidCode);
        } else if (receivedData.startsWith("GPS:")) {
          Serial.println("inn else ifff startsWith=====");  
          String gpsData = receivedData.substring(4);
          sendGPSUpdate(gpsData);
        }
  }
     Serial.println("loop end=====");
      delay(5000);  
}

// Function to send RFID check to the backend
void sendRFIDCheck (String rfidCode) {
  Serial.println("rfid====11" + rfidCode);
  if (WiFi.status() == WL_CONNECTED) {
     Serial.println("rfid====11" + rfidCode);
    HTTPClient http;
    http.begin(client, checkRFIDURL);  // Pass WiFiClient and URL
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"rfid_code\":\"" + rfidCode + "\",\"bus_no\":\"JH05DE2586\"}";
    Serial.println("rfid====11" + rfidCode);
    int httpCode = http.POST(payload);  // Send POST request
       Serial.println("rfid====22" + rfidCode);
    if (httpCode > 0) {
      String response = http.getString();
      Serial.println("RFID Check Response: " + response);  // Send response back to Arduino
    } else {
      Serial.println("Error in RFID request");
    }
    http.end();
  }
}

// Function to send GPS update to the backend
void sendGPSUpdate(String gpsData) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(client, updateLocationURL);  // Pass WiFiClient and URL
    http.addHeader("Content-Type", "application/json");

    int commaIndex = gpsData.indexOf(',');
    String latitude = gpsData.substring(0, commaIndex);
    String longitude = gpsData.substring(commaIndex + 1);

    String payload = "{\"reg_no\":\"JH05DT2024\",\"latitude\":" + latitude + ",\"longitude\":" + longitude + "}";
    int httpCode = http.POST(payload);  // Send POST request

    if (httpCode > 0) {
      String response = http.getString();
      Serial.println("GPS Update Response: " + response);  // Send response back to Arduino
    } else {
      Serial.println("Error in GPS update request");
    }
   
    http.end();
  }
}
