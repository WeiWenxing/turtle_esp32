#include "cloud.h"
#include <WiFi.h>
#include <Arduino.h>
#include <esp_log.h>

CloudService::CloudService(const char* ssid, const char* password, const char* accessToken, const char* bucketName)
    : _ssid(ssid), _password(password), _accessToken(accessToken), _bucketName(bucketName) {
}

void CloudService::uploadFile(const char* filename) {
    WiFi.begin(_ssid, _password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected");

    Serial.println("Opening file...");
    File file = SPIFFS.open(filename, "r");
    if (!file) {
        Serial.println("Failed to open file");
        return;
    }
    Serial.println("File opened successfully");

    String url = "https://storage.googleapis.com/upload/storage/v1/b/" + String(_bucketName) + "/o?uploadType=media&name=audio.wav";
    Serial.println("Connecting to URL: " + url);
    _http.begin(url);
    _http.addHeader("Authorization", "Bearer " + String(_accessToken));
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

void CloudService::receiveFile(const char* audioContent) {
    _http.begin("https://speech.googleapis.com/v1/speech:recognize");
    _http.addHeader("Content-Type", "application/json");
    _http.addHeader("Authorization", "Bearer " + String(_accessToken));
    _http.addHeader("User-Agent", "PostmanRuntime/7.40.0");
    _http.addHeader("Accept", "*/*");
    _http.addHeader("Accept-Encoding", "gzip, deflate, br");
    _http.addHeader("Connection", "keep-alive");
    _http.addHeader("x-goog-user-project", "modern-rex-420404");

    String httpRequestData = "{\"config\": {\"encoding\":\"LINEAR16\",\"languageCode\":\"en-US\",\"enableWordTimeOffsets\":false},\"audio\":{\"uri\":\"" + String(audioContent) + "\"}}";
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