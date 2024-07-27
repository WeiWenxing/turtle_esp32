#include <WiFi.h>
#include <HTTPClient.h>
#include "i2s_adc.h"
#include <FS.h>


const char* ssid = "Mangdang";
const char* password = "mangdang";
const char* accessToken = "ya29.a0AXooCgvUqfN9MgCSD5hqqGu3o-bimnqZBaC814Gr-za0NBvsbo21e9Eqxf1yvcDy3Y4KA6xJ4rl-kZu5zxOCTa_Y1CfYF4sUfsMW8Ou33Wp8FhCD-xsjrHvIjHlQf4FuenJz4_A7QcLp97pd52Er2Mm4eIomAmtItFyIsgaCgYKAfESARESFQHGX2Mimb7OwcXGGDVnDPMCVE5-dw0173";
const char filename[] = "/audio.wav";
const char* bucketName = "mangdang_voice";
const char* audioContent = "gs://mangdang_voice/audio.wav";

HTTPClient http;



void record_task(void *args) {
  record_init();
  record();
  uploadFile();
  receiveFile();
  delay(5000);
  
  vTaskDelete(NULL);
}

void uploadFile() {
  Serial.println("Opening file...");
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    Serial.println("Failed to open file");
    return;
  }
  Serial.println("File opened successfully");

  String url = "https://storage.googleapis.com/upload/storage/v1/b/" + String(bucketName) + "/o?uploadType=media&name=" + String(file.name());
  Serial.println("Connecting to URL: " + url);
  http.begin(url);
  http.addHeader("Authorization", "Bearer " + String(accessToken));
  http.addHeader("Content-Type", "application/octet-stream");

  Serial.println("Sending POST request...");
  int httpCode = http.sendRequest("POST", &file, file.size());
  Serial.println("POST request sent");

  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("Response: " + response);
  } else {
    Serial.println("Error on file upload, HTTP code: " + String(httpCode));
  }

  file.close();
  http.end();
  Serial.println("File closed and HTTP connection ended");
}

void receiveFile() {
  Serial.println("\nreceiving File\n");
  http.begin("https://speech.googleapis.com/v1/speech:recognize"); // 指定URL
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(accessToken));
  http.addHeader("User-Agent", "PostmanRuntime/7.40.0");
  http.addHeader("Accept", "*/*");
  http.addHeader("Accept-Encoding", "gzip, deflate, br");
  http.addHeader("Connection", "keep-alive");
  http.addHeader("x-goog-user-project", "modern-rex-420404");

  String httpRequestData = "{\"config\": {\"encoding\":\"LINEAR16\",\"languageCode\":\"en-US\",\"enableWordTimeOffsets\":false},\"audio\":{\"uri\":\"" + String(audioContent) + "\"}}";

  int httpResponseCode = http.POST(httpRequestData);

  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode == 200) {
    String response = http.getString();  
    Serial.println("Response: ");
    Serial.println(response);  

    int transcriptIndexStart = response.indexOf("\"transcript\": \"") + 15; 
    int transcriptIndexEnd = response.indexOf("\"", transcriptIndexStart);

    if (transcriptIndexStart > 14 && transcriptIndexEnd > transcriptIndexStart) {
      String transcript = response.substring(transcriptIndexStart, transcriptIndexEnd);
      Serial.print("Transcript: ");
      Serial.println(transcript); 
    } else {
      Serial.println("Transcript not found.");
    }
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  http.end();
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
//    uploadFile();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void loop() {
  
}
