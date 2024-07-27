#include "cloud.h"
#include <WiFi.h>
#include <Arduino.h>
#include "params.h"

const char* accessToken = "ya29.a0AXooCgvUqfN9MgCSD5hqqGu3o-bimnqZBaC814Gr-za0NBvsbo21e9Eqxf1yvcDy3Y4KAxJ4rl-kZu5zxOCTa_Y1CfYF4sUfsMW8Ou33Wp8FhCD-xsjrHvIjHlQf4FuenJz4_A7QcLp97pd52Er2Mm4eIomAmtItFyIsgaCgYKAfESARESFQHGX2Mimb7OwcXGGDVnDPMCVE5-dw0173";

const char* bucketName = "mangdang_voice";
const char* audioContent = "gs://mangdang_voice/audio.wav";
const char* gcAdd = "https://storage.googleapis.com/upload/storage/v1/b/mangdang_voice/o?uploadType=media&name=audio.wav";

HTTPClient _http;

void uploadFile() {
    Serial.println("Opening file...");
    File file = SPIFFS.open(filename, "r");
    if (!file) {
        Serial.println("Failed to open file");
        return;
    }
    Serial.println("File opened successfully");

    // String url = "https://storage.googleapis.com/upload/storage/v1/b/" + String(_bucketName) + "/o?uploadType=media&name=audio.wav";
    Serial.print("Connecting to URL: ");
    Serial.println(gcAdd);
    _http.begin(gcAdd);
    _http.addHeader("Authorization", "Bearer " + String(accessToken));
    _http.addHeader("Content-Type", "application/octet-stream");

    Serial.println("Sending POST request...");
    int httpCode = _http.sendRequest("POST", &file, file.size());
    Serial.println("POST request sent");

    if (httpCode > 0) {
        String response = _http.getString();
        Serial.println("Response: " + response);
    } else {
        Serial.println("Error on file upload, HTTP code: " + String(httpCode));
    }

    file.close();
    _http.end();
    Serial.println("File closed and HTTP connection ended");
}

void speechToText() {
    _http.begin("https://speech.googleapis.com/v1/speech:recognize");
    _http.addHeader("Content-Type", "application/json");
    _http.addHeader("Authorization", "Bearer " + String(accessToken));
    _http.addHeader("User-Agent", "PostmanRuntime/7.40.0");
    _http.addHeader("Accept", "*/*");
    _http.addHeader("Accept-Encoding", "gzip, deflate, br");
    _http.addHeader("Connection", "keep-alive");
    _http.addHeader("x-goog-user-project", "modern-rex-420404");

    String httpRequestData = "{\"config\": {\"encoding\":\"LINEAR16\",\"languageCode\":\"en-US\",\"enableWordTimeOffsets\":false},\"audio\":{\"uri\":\"gs://mangdang_voice/audio.wav\"}}";
    int httpResponseCode = _http.POST(httpRequestData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
        String response = _http.getString();  
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
    _http.end();
}