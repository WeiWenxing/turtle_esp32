#include <WiFi.h>
#include <HTTPClient.h>
#include <FS.h>
#include "i2s_adc.h"
#include "tts.h"

const char* ssid = "Mangdang";
const char* password = "mangdang";

unsigned long cloud_start_time, gc_end_time, stt_end_time, ai_end_time, duration;  // for delay

void record_task(void* args) {
  while (1) {
    Serial.println("=================================Record start!=================================");
    record();
    Serial.println("Record end!");

    Serial.println("=================================Speech start!=================================");
    tts("Hi, what can I do for you today!");
  }
  // delay(5000);
  // vTaskDelete(NULL);
}


void setup() {
  Serial.begin(115200);
  Serial.println("Starting setup...");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected");

  if (WiFi.status() == WL_CONNECTED) {
    xTaskCreate(record_task, "record_task", 1024 * 8, NULL, 1, NULL);
    // record_task((void*)NULL);
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void loop() {
}
