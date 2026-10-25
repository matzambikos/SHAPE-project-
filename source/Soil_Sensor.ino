/****************************************************
* Adafruit ESP32-S3 Reverse TFT Feather
* HW-101 Soil Moisture Logger to Supabase
* Updates every 5 seconds
****************************************************/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/************** USER SETTINGS ***************/
// ---- Wi-Fi ----
const char* ssid     = "Columbia University";  // <<-- Put your Wi-Fi network name here
const char* password = "";                // <<-- Leave empty if no password

// ---- Supabase ----
const char* supabase_url = "https://hlfjotqhfzkcrbmvpdzq.supabase.co"; 
// <<-- Replace with your Supabase REST API endpoint

const char* supabase_api_key = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImhsZmpvdHFoZnprY3JibXZwZHpxIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NTQzMzMxNzMsImV4cCI6MjA2OTkwOTE3M30.zEWorM6qdbsoSaajn5Sb9WLj_uZsmFUy-EY8-OC-LGI"; 
// <<-- Put your Supabase API key here

// ---- SSL (Optional) ----
// If using HTTPS with SSL verification, paste your root certificate here
// const char* root_ca = R"EOF(
// -----BEGIN CERTIFICATE-----
// PASTE_YOUR_SSL_CERT_HERE
// -----END CERTIFICATE-----
// )EOF;

/************** SENSOR SETTINGS ***************/
const int soilPin = A0; // On this Feather, A0 = GPIO1

// Optional calibration values for mapping to % moisture
int dryValue = 3500; // Adjust after measuring dry soil reading
int wetValue = 1200; // Adjust after measuring wet soil reading

/************** LOOP TIMING ***************/
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 5000; // 5 seconds

void setup() {
 Serial.begin(115200);
 delay(1000);

 analogReadResolution(12); // 0–4095 range

 Serial.println("Connecting to Wi-Fi...");
 WiFi.begin(ssid, password);

 while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
 }
 Serial.println("\nWi-Fi connected!");
 Serial.print("IP Address: ");
 Serial.println(WiFi.localIP());
}

void loop() {
 if (millis() - lastUpdate >= updateInterval) {
   lastUpdate = millis();

   // Read soil moisture raw value
   int soilRaw = analogRead(soilPin);

   // Map to % moisture (0% = dry, 100% = wet)
   int moisturePercent = map(soilRaw, dryValue, wetValue, 0, 100);
   moisturePercent = constrain(moisturePercent, 0, 100);

   Serial.print("Soil raw: ");
   Serial.print(soilRaw);
   Serial.print(" | Moisture: ");
   Serial.print(moisturePercent);
   Serial.println("%");

   // Create JSON payload
   StaticJsonDocument<200> doc;
   doc["soil_moisture_raw"] = soilRaw;
   doc["soil_moisture_percent"] = moisturePercent;
   doc["temperature"] = nullptr;    // Placeholder for future sensor
   doc["light_intensity"] = nullptr; // Placeholder for future sensor

   String jsonData;
   serializeJson(doc, jsonData);

   // Send to Supabase
   if (WiFi.status() == WL_CONNECTED) {
     HTTPClient http;
     // If SSL verification needed: http.begin(supabase_url, root_ca);
     http.begin(supabase_url);
     http.addHeader("Content-Type", "application/json");
     http.addHeader("apikey", supabase_api_key);
     http.addHeader("Authorization", String("Bearer ") + supabase_api_key);

     int httpResponseCode = http.POST(jsonData);

     if (httpResponseCode > 0) {
       Serial.print("Supabase Response Code: ");
       Serial.println(httpResponseCode);
       Serial.println(http.getString());
     } else {
       Serial.print("Error sending POST: ");
       Serial.println(http.errorToString(httpResponseCode));
     }

     http.end();
   } else {
     Serial.println("Wi-Fi disconnected! Reconnecting...");
     WiFi.begin(ssid, password);
   }
 }
}


