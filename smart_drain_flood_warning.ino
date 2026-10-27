/*
  FINAL IoT-Based Water Level and Drain Blockage Monitoring System
  Board: ESP32 DevKit V1 (30-Pin)
  Sensor: JSN-SR04T Waterproof Ultrasonic Sensor
  Includes Direct TCP + DNS Bypass for HTTP Code -1 Fix
*/

#include <WiFi.h>
#include <HTTPClient.h>

// ================= 1. YOUR WIFI & THINGSPEAK DETAILS =================
const char* ssid = " ";                        // Your 2.4GHz Hotspot Name
const char* password = " ";               // Your Hotspot Password
const char* thingSpeakApiKey = " "; // Your Write API Key

// Official ThingSpeak Host & Static IP (Bypasses Hotspot DNS Failure)
const char* thingSpeakHost = "api.thingspeak.com";
IPAddress thingSpeakIP(184, 106, 153, 149);

// ================= 2. CONTAINER HEIGHT SETTINGS =================
const float SENSOR_MOUNT_HEIGHT_CM = 40.0; // Total distance from sensor face to bottom
const float MAX_WATER_DEPTH_CM     = 20.0; // Max rise of water

// ================= 3. EXACT PIN DEFINITIONS =================
const int TRIG_PIN   = 12; // Sensor TRIG (Hole J04)
const int ECHO_PIN   = 14; // Sensor ECHO via Voltage Divider (Hole J05)
const int GREEN_LED  = 27; // Normal LED (Hole J06)
const int YELLOW_LED = 26; // Warning LED (Hole J07)
const int RED_LED    = 25; // Critical LED (Hole J08)
const int BUZZER_PIN = 33; // Active Buzzer (Hole J09)

unsigned long lastCloudUpload = 0;
const unsigned long UPLOAD_INTERVAL_MS = 16000; // 16 seconds between ThingSpeak updates

// Remember last good reading so blind-zone spikes never stop ThingSpeak uploads
float lastValidDistance = 20.0;

void connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("\nConnecting to Hotspot: ");
  Serial.println(ssid);
  
  WiFi.disconnect(true);
  delay(200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    // Force Google DNS (8.8.8.8) so mobile hotspots don't fail DNS lookup (-1 error)
    WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), IPAddress(8, 8, 8, 8), IPAddress(8, 8, 4, 4));
    Serial.println("\n[SUCCESS] Wi-Fi Connected!");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[ERROR] Could not connect to 'GGG'. Check Hotspot is ON and set to 2.4GHz.");
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Startup Hardware Self-Test
  Serial.println("\n--- RUNNING HARDWARE SELF-TEST ---");
  digitalWrite(GREEN_LED, HIGH);  delay(400); digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH); delay(400); digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, HIGH);    digitalWrite(BUZZER_PIN, HIGH); delay(400);
  digitalWrite(RED_LED, LOW);     digitalWrite(BUZZER_PIN, LOW);
  Serial.println("--- SELF-TEST COMPLETE ---");

  connectToWiFi();

  // Trigger first upload immediately
  lastCloudUpload = millis() - UPLOAD_INTERVAL_MS;
}

float getStableDistanceCM() {
  float sum = 0;
  int count = 0;

  for (int i = 0; i < 5; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(20);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 35000);
    float dist = duration * 0.0343 / 2.0;

    // Accept slightly wider margin (15cm to 55cm) so top-level water doesn't cut out
    if (dist >= 15.0 && dist <= (SENSOR_MOUNT_HEIGHT_CM + 15.0)) {
      sum += dist;
      count++;
    }
    delay(40);
  }

  if (count == 0) return -1.0;
  return sum / count;
}

// Bulletproof Upload Function: Uses Direct TCP Socket (Host + Static IP Fallback)
void uploadToThingSpeak(float waterLevel, float distance, int status) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ThingSpeak] Wi-Fi disconnected! Reconnecting...");
    connectToWiFi();
  }

  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClient client;
  client.setTimeout(10000); // 10 second timeout for slow mobile data

  Serial.println("\n>>> Uploading to ThingSpeak...");

  // Try connecting by domain name first; if hotspot DNS fails, connect directly via Static IP!
  bool connected = client.connect(thingSpeakHost, 80);
  if (!connected) {
    Serial.println("[Info] DNS slow on hotspot, connecting via ThingSpeak Direct IP...");
    connected = client.connect(thingSpeakIP, 80);
  }

  if (connected) {
    String postStr = String("api_key=") + thingSpeakApiKey +
                     "&field1=" + String(waterLevel, 1) +
                     "&field2=" + String(distance, 1) +
                     "&field3=" + String(status);

    client.print("POST /update HTTP/1.1\r\n");
    client.print("Host: api.thingspeak.com\r\n");
    client.print("Connection: close\r\n");
    client.print("Content-Type: application/x-www-form-urlencoded\r\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\r\n\r\n");
    client.print(postStr);

    // Wait for ThingSpeak server reply
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 7000) {
        Serial.println("[ThingSpeak ERROR] Server timed out. Check that Mobile Data is ON!");
        client.stop();
        return;
      }
    }

    // Read response body (Entry ID is the last line)
    String lastLine = "";
    while (client.available()) {
      String line = client.readStringUntil('\n');
      line.trim();
      if (line.length() > 0) {
        lastLine = line;
      }
    }
    client.stop();

    if (lastLine.toInt() > 0) {
      Serial.print("[ThingSpeak SUCCESS] Charts Updated! Entry ID #");
      Serial.println(lastLine);
    } else {
      Serial.print("[ThingSpeak Server Reply]: ");
      Serial.println(lastLine);
    }
  } else {
    Serial.println("[ThingSpeak ERROR] Cannot reach internet! Make sure Mobile Data is turned ON on phone 'GGG'.");
  }
}

void loop() {
  float distance = getStableDistanceCM();

  // If water gets closer than 20cm (into the blind zone), clamp it to 20.0cm instead of skipping!
  if (distance < 0) {
    Serial.println("[Note] Sensor in blind zone (<20cm) -> Clamping to last valid level so charts keep updating.");
    distance = lastValidDistance;
  } else {
    lastValidDistance = distance;
  }

  // Calculate Water Level (Depth) and Fill Percentage
  float waterLevel = SENSOR_MOUNT_HEIGHT_CM - distance;
  if (waterLevel < 0) waterLevel = 0;
  if (waterLevel > MAX_WATER_DEPTH_CM) waterLevel = MAX_WATER_DEPTH_CM;

  float fillPercent = (waterLevel / MAX_WATER_DEPTH_CM) * 100.0;
  int statusCode = 1; // 1 = Normal, 2 = Warning, 3 = Critical

  // Instant Local LED & Buzzer Control
  if (fillPercent < 45.0) {
    statusCode = 1;
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  } 
  else if (fillPercent >= 45.0 && fillPercent < 75.0) {
    statusCode = 2;
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  } 
  else {
    statusCode = 3;
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    
    // Short alert beep
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Print Live Readings to Serial Monitor
  Serial.print("Water Level (Field 1): ");
  Serial.print(waterLevel, 1);
  Serial.print(" cm | Distance (Field 2): ");
  Serial.print(distance, 1);
  Serial.print(" cm | Status (Field 3): ");
  Serial.println(statusCode);

  // Upload to ThingSpeak every 16 seconds
  if (millis() - lastCloudUpload >= UPLOAD_INTERVAL_MS) {
    uploadToThingSpeak(waterLevel, distance, statusCode);
    lastCloudUpload = millis();
  }

  delay(800);
}
