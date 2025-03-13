#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Fingerprint.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "FPTU_Library";
const char* password = "12345678";

// Google Apps Script URL
const char* googleScriptURL = "https://script.google.com/macros/s/YOUR_SCRIPT_ID/exec";

// Fingerprint sensor setup
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    finger.begin(57600);
    if (finger.verifyPassword()) {
        Serial.println("Fingerprint sensor detected");
    } else {
        Serial.println("Fingerprint sensor not detected");
        logFingerprintToSheetsAndDocs("", "Error", "Fingerprint sensor not detected");
        while (1);
    }
}

void loop() {
    Serial.println("Waiting for valid fingerprint...");
    int result = finger.getImage();
    if (result == FINGERPRINT_OK) {
        if (finger.image2Tz(1) == FINGERPRINT_OK) {
            if (finger.createModel() == FINGERPRINT_OK) {
                Serial.println("Fingerprint template created.");
                
                // Convert fingerprint to HEX string
                String fingerprintTemplate = getFingerprintTemplate();
                logFingerprintToSheetsAndDocs(fingerprintTemplate, "Success", "Fingerprint scanned and stored successfully");
            } else {
                Serial.println("Failed to create fingerprint template");
                logFingerprintToSheetsAndDocs("", "Error", "Failed to create fingerprint template");
            }
        } else {
            Serial.println("Failed to convert image to template");
            logFingerprintToSheetsAndDocs("", "Error", "Failed to convert fingerprint image");
        }
    } else if (result == FINGERPRINT_NOFINGER) {
        Serial.println("No finger detected");
    } else {
        Serial.println("Fingerprint read error");
        logFingerprintToSheetsAndDocs("", "Error", "Fingerprint sensor read error");
    }
    delay(5000);
}

String getFingerprintTemplate() {
    uint8_t templateBuffer[512];
    if (finger.getTemplate(1, templateBuffer) == FINGERPRINT_OK) {
        String templateStr = "";
        for (int i = 0; i < 512; i++) {
            templateStr += String(templateBuffer[i], HEX);
        }
        return templateStr;
    }
    return "";
}

void logFingerprintToSheetsAndDocs(String fingerprintTemplate, String status, String message) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        WiFiClientSecure client;
        client.setInsecure();
        
        http.begin(client, googleScriptURL);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        
        String postData = "fingerprint_template=" + fingerprintTemplate + "&status=" + status + "&message=" + message;
        int httpResponseCode = http.POST(postData);
        
        Serial.print("Google API response: ");
        Serial.println(httpResponseCode);
        
        http.end();
    }
}
