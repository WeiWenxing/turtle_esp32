#include <WiFi.h>
#include <HTTPClient.h>
#include <FS.h>
#include "i2s_adc.h"
#include "cloud.h"
#include "tts.h"

const char* ssid = "Mangdang";
const char* password = "mangdang";


void record_task(void* args) {
  record_init();
  record();
  uploadFile();
  String input_text = speechToText();
  if (input_text != "") {
    String ai_text = aitts(input_text);
    if (ai_text != "") {
      tts(ai_text);
    }
  }
  // tts("I  am doing well, thank you for asking! I am a large language model,  so I don't have a name in the traditional sense. You can call me Amy");
  delay(5000);

  vTaskDelete(NULL);
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
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void loop() {
}
