#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid   = "YOUR_WIFI_SSID";
const char* pass   = "YOUR_WIFI_PASSWORD";
const char* apiKey = "YOUR_THINGSPEAK_WRITE_KEY";

#define TRIG  5
#define ECHO  18
#define LED_G 25
#define LED_Y 26
#define LED_R 27
#define BUZZ  14

const float H = 40.0;      // sensor height above channel bottom (cm)
const float DMAX = 15.0;   // depth treated as 100 % (cm)
const float WARN = 40.0, CRIT = 70.0;
unsigned long lastUpload = 0;

float readDistance() {
  digitalWrite(TRIG, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long t = pulseIn(ECHO, HIGH, 30000);
  return (t == 0) ? -1 : t * 0.0343 / 2;
}

float medianDistance() {
  float v[5];
  for (int i = 0; i < 5; i++) { v[i] = readDistance(); delay(60); }
  for (int i = 0; i < 4; i++)
    for (int j = i + 1; j < 5; j++)
      if (v[j] < v[i]) { float t = v[i]; v[i] = v[j]; v[j] = t; }
  return v[2];
}

void setup() {
  Serial.begin(115200);                       // added
  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);
  pinMode(LED_G, OUTPUT); pinMode(LED_Y, OUTPUT);
  pinMode(LED_R, OUTPUT); pinMode(BUZZ, OUTPUT);
  WiFi.begin(ssid, pass);
}

void loop() {
  float d = medianDistance();
  if (d > 0) {
    float level = constrain((H - d) / DMAX * 100.0, 0, 100);
    int status = (level >= CRIT) ? 2 : (level >= WARN) ? 1 : 0;
    Serial.printf("d=%.1f cm  level=%.1f %%  status=%d\n", d, level, status);  // added

    digitalWrite(LED_G, status == 0);
    digitalWrite(LED_Y, status == 1);
    digitalWrite(LED_R, status == 2);
    digitalWrite(BUZZ,  status == 2);

    if (WiFi.status() == WL_CONNECTED && millis() - lastUpload > 15000) {
      HTTPClient http;
      String url = String("http://api.thingspeak.com/update?api_key=") + apiKey
                   + "&field1=" + level + "&field2=" + d + "&field3=" + status;
      http.begin(url);
      http.GET();
      http.end();
      lastUpload = millis();
    }
  }
  delay(500);
}

