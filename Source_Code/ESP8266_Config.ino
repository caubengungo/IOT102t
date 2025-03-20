#include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>

  const char* ssid = "Chipichipichapachapa"; 
  const char* password = "chuachacdafreedau"; 
  const char* scriptURL = "https://script.google.com/macros/s/AKfycbzvveHuWtFZFu-YjR1tQ4W4KsWogfneNfX0tsqF8fkc98yb8y2bYzCR-RlBzxugU8Pfhw/exec";

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
      encodedData.replace(" ", "%20");  // Encode space character
      encodedData.replace("&", "%26");  // Encode &

      // String url = String(scriptURL) + "?" + encodedData;
      String url = String(scriptURL) + encodedData;
      http.begin(client, url);
      client.setInsecure(); // Skip SSL testing
      // String url = String(scriptURL) + "?" + data;
      Serial.println("Requesting URL: " + url);
      
      Serial.print("Free heap memory: ");
      Serial.println(ESP.getFreeHeap());
      http.begin(client, url);
      // http.addHeader("Connection", "close"); // Fix error HTTP/1.0
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      int httpCode = http.GET();

          // if get redirect (302), get new URL and send again
      if (httpCode == HTTP_CODE_FOUND) { // 302 Redirect
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
