#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// Define multiple Wi-Fi credentials
const char* ssids[] = {"JioFiber-84KxT","realme9","POCOM2Pro"};
const char* passwords[] = {"AhUT1836#@","b3996mue","1223334444"};
const int numNetworks = sizeof(ssids) / sizeof(ssids[0]);

const char* apiUrl = "https://bus-arka-server.vercel.app/api/bus_location/update";
const char* apiUrlGET = "https://jsonplaceholder.typicode.com/todos/1";

unsigned long previousMillis = 0;
const long interval = 5000;

String stud_rfid = "";
String stud_busno = "";

String busno = "";
double bus_lat=0.0;
double bus_lng=0.0;
#define WIFI_LED 2  // Onboard LED (GPIO2)

int getJsonStatusCode2(String response) {
  // Assuming the response is a string that includes JSON data
  int jsonStart = response.indexOf("{");  // Find the start of the JSON
  int jsonEnd = response.indexOf("}");    // Find the end of the JSON
  
  if (jsonStart == -1 || jsonEnd == -1) {
    Serial.println("Error: JSON not found");
    return -1;
  }
  
  // Extract the JSON string
  String jsonPart = response.substring(jsonStart, jsonEnd + 1);  // Including the curly braces
  
  // Parse the JSON manually or use a library like ArduinoJson
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, jsonPart);
  int statusCode = doc["statusCode"];
  
  return statusCode;
}


void blinkLEDSlow() {
  // Slow blink pattern (500ms ON, 500ms OFF)
  digitalWrite(WIFI_LED, HIGH);
  delay(500);
  digitalWrite(WIFI_LED, LOW);
  delay(500);
}

void blinkLEDFast() {
  // Fast blink pattern (200ms ON, 200ms OFF)
  digitalWrite(WIFI_LED, HIGH);
  delay(200);
  digitalWrite(WIFI_LED, LOW);
  delay(200);
}


// Function to connect to Wi-Fi
bool connectToWiFi() {
  for (int i = 0; i < numNetworks; i++) {
    WiFi.begin(ssids[i], passwords[i]);
    unsigned long startAttemptTime = millis();

    // Wait for connection or timeout after 10 seconds
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      blinkLEDSlow();
      delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
       digitalWrite(WIFI_LED, LOW);
      return true; // Connected successfully
    }
  }

blinkLEDFast();
  return false; // Failed to connect
}




void setup() {
  Serial.begin(9600);
  pinMode(WIFI_LED, OUTPUT);
  delay(10);
  if (!connectToWiFi()) {
      // If Wi-Fi connection fails, blink LED quickly
      blinkLEDFast();
    } else {
      // If Wi-Fi is connected, turn off the LED
      digitalWrite(WIFI_LED, LOW);
    }
  }

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
//    Serial.println("Wi-Fi connection lost. Attempting to reconnect...");
    if (!connectToWiFi()) {
//      Serial.println("Reconnection failed. Retrying...");
      delay(100);
    }
  }
//  Serial.println("WiFi connected!");
//  Serial.println("IP Address: ");
//  Serial.println(WiFi.localIP());

  if (Serial.available()) {
    //  Serial.println("inside mySerial===");
    String data = Serial.readString(); // Read data from Arduino
    // Serial.println("Received data from UNO:" + data);

    // STUDENT RFID REQUEST START
   if (data.indexOf("STUD_RFID::") != -1 && data.indexOf("STUD_BUSNO::") != -1) {
        
      int rfidStart = data.indexOf("STUD_RFID::") + String("STUD_RFID::").length();
      int rfidEnd = data.indexOf(",STUD_BUSNO::");
      if (rfidStart > 0 && rfidEnd > rfidStart) {
          String temp = data.substring(rfidStart, rfidEnd); // Extract the substring
          temp.trim(); // Trim in place
          stud_rfid = temp; // Assign to stud_rfid
      }
  
      int busNoStart = data.indexOf("STUD_BUSNO::") + String("STUD_BUSNO::").length();
      int busNoEnd = data.indexOf(";", busNoStart); // Find the index of the semicolon
      if (busNoStart > 0 && busNoEnd > busNoStart) {
          String temp = data.substring(busNoStart, busNoEnd); // Extract the substring
          temp.trim(); // Trim in place
          stud_busno = temp; // Assign to stud_busno
      }       
      // Check if both RFID and Bus Number are valid
      if (stud_rfid != "" && stud_busno != "") {
          // Process the values
          sendcURLStudentLoginPostRequest2(stud_rfid, stud_busno); // student API call
      } else {
           Serial.println("STUD_RESPO::601;");
      }
  }

   // STUDENT RFID REQUEST END

    // BUS LOCATION UPDATE START
  if (data.indexOf("BUS_NO::") != -1 && data.indexOf("GPS_LAT::") != -1 && data.indexOf("GPS_LNG::") != -1) {
      String busno="";
        // Extract Bus Number
        int busnoStart = data.indexOf("BUS_NO::") + String("BUS_NO::").length();
        int busnoEnd = data.indexOf(",GPS_LAT::");
        if (busnoStart > 0 && busnoEnd > busnoStart) {
            busno = data.substring(busnoStart, busnoEnd);
        }
        
        // Extract GPS Latitude
        int gpsLatStart = data.indexOf("GPS_LAT::") + String("GPS_LAT::").length();
        int gpsLatEnd = data.indexOf(",GPS_LNG::");
        if (gpsLatStart > 0 && gpsLatEnd > gpsLatStart) {
            bus_lat = data.substring(gpsLatStart, gpsLatEnd).toDouble();
        }

        int gpsLngStart = data.indexOf("GPS_LNG::") + String("GPS_LNG::").length();
        int gpsLngEnd = data.indexOf(";", gpsLngStart); // Find the index of the semicolon
        
        if (gpsLngStart > 0 && gpsLngEnd > gpsLngStart) {
            String lngStr = data.substring(gpsLngStart, gpsLngEnd); // Extract the longitude part
            lngStr.trim(); // Trim the string
            bus_lng = lngStr.toDouble(); // Convert the trimmed string to double
        } else if (gpsLngStart > 0) {
            String lngStr = data.substring(gpsLngStart); // Extract until the end of the string
            lngStr.trim(); // Trim the string
            bus_lng = lngStr.toDouble(); // Convert the trimmed string to double
        }

        
      if (bus_lat != 0.0 && bus_lng != 0.0 && busno != "") {
          // Process the values
          sendcURLBusLocationPostRequest2(busno, String(bus_lat, 10), String(bus_lng, 10));  // Convert double to String with 10 decimal places
          
      } else {
//          Serial.println("BUS_RESPO::601");
      }
    }// BUS LOCATION UPDATE START
    
  }
  // Serial.println("loop..");
  // sendcURLStudentLoginPostRequest2("7a211902", "JH05DT2024");
  // sendcURLBusLocationPostRequest2("JH05DT6619", "22.809681", "86.102898");
  // delay(30000);
}

///new cpde 
void sendcURLStudentLoginPostRequest2(String stud_rfid, String stud_busno) {
//  Serial.println("Call sendcURLStudentLoginPostRequest=====");

  // Define API URL and setup client
  const char* url = "https://bus-arka-server.vercel.app/api/rfid/check";

  // JSON payload for the POST request
  String payload = "{\"rfid_code\":\"" + stud_rfid + "\",\"reg_no\":\"" + stud_busno + "\"}";
//  Serial.println(payload);
  // Use WiFiClientSecure for HTTPS
  WiFiClientSecure client;
  client.setInsecure();  // Disable SSL certificate validation

  HTTPClient https;  // Create HTTP client object

  // Begin HTTP request
  if (https.begin(client, url)) {
    // Add headers
    https.addHeader("Content-Type", "application/json");

    // Send POST request and receive response code
    int httpResponseCode = https.POST(payload);

    // Handle the response
    if (httpResponseCode > 0) {
      // HTTP request was successful
      String response = https.getString();  // Get the server's response
      // Extract the statusCode from the JSON response
      int statusCode = getJsonStatusCode2(response);  // Assuming getJsonStatusCode2 parses JSON correctly
      if (statusCode != -1) {
        Serial.println("STUD_RESPO::" + String(statusCode) +";");
      } else {
        Serial.println("STUD_RESPO::600;");
      }

    } else {
      // HTTP request failed
      Serial.println("Error on sending POST: " + String(httpResponseCode));
      Serial.println("STUD_RESPO::600;");
    }

    // End the HTTP connection
    https.end();
  } else {
    // Connection to the server failed
    Serial.println("Unable to connect to server");
    Serial.println("STUD_RESPO::600;");
  }
}

//new code 
void sendcURLBusLocationPostRequest2(String busno, String bus_lat_str, String bus_lng_str) {
//  Serial.println("Call sendcURLBusLocationPostRequest=====");

  // Define API URL
  const char* url = "https://bus-arka-server.vercel.app/api/bus_location/update";

  // Convert String to double for latitude and longitude
  double bus_lat = bus_lat_str.toDouble();
  double bus_lng = bus_lng_str.toDouble();

  // Convert latitude and longitude to char arrays with 6 decimal places
  char lat_str[20];
  char lng_str[20];
  dtostrf(bus_lat, 5, 6, lat_str);  // 5 total characters (including decimals), 6 decimal places
  dtostrf(bus_lng, 5, 6, lng_str);  // 5 total characters (including decimals), 6 decimal places

  // Prepare JSON payload
  String payload = "{\"reg_no\":\"" + busno + "\",\"latitude\":" + lat_str + ",\"longitude\":" + lng_str + "}";
  
  // Use WiFiClientSecure for HTTPS connection
  WiFiClientSecure client;
  client.setInsecure();  // Disable SSL certificate verification

  HTTPClient https;  // Create HTTP client object

  // Begin HTTP request
  if (https.begin(client, url)) {
    // Add headers
    https.addHeader("Content-Type", "application/json");
//    Serial.println("payload: " + payload);
    // Send POST request and receive response code
    int httpResponseCode = https.POST(payload);

    // Handle the response
    if (httpResponseCode > 0) {

      String response = https.getString();  // Get the server's response

      // Extract the statusCode from the JSON response
      int statusCode = getJsonStatusCode2(response);  // Assuming getJsonStatusCode2 parses JSON correctly
      if (statusCode != -1) {
//        Serial.println("BUS_RESPO::" + String(statusCode));
      } else {
//        Serial.println("BUS_RESPO::600");
      }
    } else {
      // HTTP request failed
      Serial.println("Error on sending POST: " + String(httpResponseCode));
//      Serial.println("BUS_RESPO::600");
    }

    // End the HTTP connection
    https.end();
  } else {
    // Connection to the server failed
    Serial.println("Unable to connect to server");
//    Serial.println("BUS_RESPO::600");
  }
}