/*
 * Smart Fan System - Temperature Controlled
 * Hardware: Arduino Nano + ESP8266 + BME280 + Relay Module + LCD 16x2
 * Author: Aytunga Team
 */

#include <Wire.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// ─── PIN DEFINITIONS ───────────────────────────────────────────────
#define RELAY_PIN       7       // Relay IN pin
#define ESP_RX          10      // Arduino RX ← ESP8266 TX
#define ESP_TX          11      // Arduino TX → ESP8266 RX

// ─── THRESHOLDS ────────────────────────────────────────────────────
#define TEMP_ON         28.0    // °C → fan turns ON
#define TEMP_OFF        25.0    // °C → fan turns OFF (hysteresis)

// ─── WIFI CONFIG (edit before upload) ──────────────────────────────
const char* WIFI_SSID     = "YOUR_SSID";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";
// ThingSpeak (free IoT cloud)
const char* TS_API_KEY    = "YOUR_THINGSPEAK_WRITE_KEY";
const char* TS_HOST       = "api.thingspeak.com";

// ─── OBJECTS ───────────────────────────────────────────────────────
Adafruit_BME280   bme;
LiquidCrystal_I2C lcd(0x27, 16, 2);   // I2C addr 0x27 (try 0x3F if blank)
SoftwareSerial    espSerial(ESP_RX, ESP_TX);

// ─── STATE ─────────────────────────────────────────────────────────
bool fanState       = false;
unsigned long lastUpload = 0;
const unsigned long UPLOAD_INTERVAL = 60000; // 60 s

// ─── HELPERS ───────────────────────────────────────────────────────
void espSend(const String& cmd, unsigned int waitMs = 1000) {
  espSerial.println(cmd);
  delay(waitMs);
  while (espSerial.available()) Serial.write(espSerial.read());
}

void connectWiFi() {
  espSend("AT+RST", 2000);
  espSend("AT+CWMODE=1");
  espSend("AT+CWJAP=\"" + String(WIFI_SSID) + "\",\"" + String(WIFI_PASSWORD) + "\"", 5000);
  Serial.println(F("[WiFi] Connected (hopefully)"));
}

void uploadToCloud(float temp, float hum, float pres) {
  String data = "field1=" + String(temp, 1) +
                "&field2=" + String(hum, 1)  +
                "&field3=" + String(pres, 1);

  espSend("AT+CIPSTART=\"TCP\",\"" + String(TS_HOST) + "\",80", 2000);

  String httpReq =
    "GET /update?api_key=" + String(TS_API_KEY) + "&" + data + " HTTP/1.1\r\n"
    "Host: " + String(TS_HOST) + "\r\n"
    "Connection: close\r\n\r\n";

  espSend("AT+CIPSEND=" + String(httpReq.length()), 1000);
  espSend(httpReq, 2000);
  espSend("AT+CIPCLOSE");
  Serial.println(F("[Cloud] Data uploaded"));
}

void setFan(bool on) {
  fanState = on;
  digitalWrite(RELAY_PIN, on ? LOW : HIGH);  // LOW = relay ON (active-low)
}

void updateLCD(float temp, float hum) {
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temp, 1);
  lcd.print((char)223); // degree symbol
  lcd.print("C H:");
  lcd.print((int)hum);
  lcd.print("%  ");

  lcd.setCursor(0, 1);
  lcd.print("Fan: ");
  lcd.print(fanState ? "ON  " : "OFF ");
  lcd.print(fanState ? (char)0 : ' ');  // custom char (optional)
}

// ─── SETUP ─────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  espSerial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // fan OFF at start (active-low relay)

  lcd.init();
  lcd.backlight();
  lcd.print(F("Smart Fan v1.0"));

  if (!bme.begin(0x76)) {         // try 0x77 if 0x76 fails
    Serial.println(F("[ERROR] BME280 not found!"));
    lcd.setCursor(0, 1);
    lcd.print(F("BME280 ERROR!"));
    while (true);
  }

  Serial.println(F("[OK] BME280 found"));
  connectWiFi();
  delay(1000);
  lcd.clear();
}

// ─── LOOP ──────────────────────────────────────────────────────────
void loop() {
  float temp = bme.readTemperature();
  float hum  = bme.readHumidity();
  float pres = bme.readPressure() / 100.0F; // hPa

  Serial.print(F("[Sensor] T="));  Serial.print(temp);
  Serial.print(F(" H="));          Serial.print(hum);
  Serial.print(F(" P="));          Serial.println(pres);

  // ── Fan control (hysteresis) ──
  if (!fanState && temp >= TEMP_ON)  setFan(true);
  if ( fanState && temp <= TEMP_OFF) setFan(false);

  updateLCD(temp, hum);

  // ── Cloud upload every 60 s ──
  if (millis() - lastUpload >= UPLOAD_INTERVAL) {
    uploadToCloud(temp, hum, pres);
    lastUpload = millis();
  }

  delay(2000); // read every 2 s
}
