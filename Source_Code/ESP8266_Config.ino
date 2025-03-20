  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>

  const char* ssid = "TQT"; 
  const char* password = "calculus"; 
  const char* scriptURL = "https://script.google.com/macros/s/AKfycbzzpflQzsa1GYSUMcKCNhVHdC6Z4Lo9W2fqmthB1ZcrBdyezoN6Gh2f4O9e0seGCCvyAg/exec";

  void setup() {
    Serial.begin(9600);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
  }

  void loop() {
    if (Serial.available()) {
      String logData = Serial.readStringUntil('\n');
      sendToGoogleSheets(logData);
    }
  }

  void sendToGoogleSheets(String data) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      HTTPClient http;

      String encodedData = data;
      encodedData.replace(" ", "%20");  // Encode the space
      encodedData.replace("&", "%26");  // Encode the symbol "&"

      String url = String(scriptURL) + encodedData;
      http.begin(client, url);
      client.setInsecure(); // Skip the SSL testing
      Serial.println("Requesting URL: " + url);
      
      Serial.print("Free heap memory: ");
      Serial.println(ESP.getFreeHeap());
      http.begin(client, url);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      int httpCode = http.GET();

      // If it get Redirected, get the new URL then send again
      if (httpCode == HTTP_CODE_FOUND) {
        String newURL = http.getLocation();
        Serial.println("Redirect to: " + newURL);
        http.end();
        
        http.begin(client, newURL);
        httpCode = http.GET();
      }

      Serial.print("HTTP Response Code: ");
      Serial.println(httpCode);
      if (httpCode == HTTP_CODE_OK) {
        Serial.println("Data sent successfully");
      } else {
        Serial.println("Error sending data");
      }
      http.end();
    }
  }
