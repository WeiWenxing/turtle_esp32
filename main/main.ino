#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "i2s_adc.h"

const char* ssid = "Mangdang";
const char* password = "mangdang";

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Hello world");
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void loop() {
  // 主循环中不做任何事情
}