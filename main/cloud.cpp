#include "cloud.h"
#include <WiFi.h>
#include <Arduino.h>
#include "params.h"
#include "Audio.h"

const char* accessToken = "ya29.c.c0ASRK0GYV7G7uZtlfKNw9c0glPGNjooryn0DRjcI0XzSu_1PARXoc8aImTVYLzDnej8dQ0BywQFG-dV565CPK-x3bASPZgok71yC3rDThmo9-0XOG-bTgxW-VDP5oxAeR8BANhUgDHaLKBvPzCvcZzGnKeLfI_r3J1_RpGyPhJOOr70X0rgP6qDLNL-0vkWh9lVH0cknSk1uuk9nerrmeRWEPb1jcjp8On1KvB-NbgaF27NDKLnvr5Zp7b32si_4pK9Y4fxpn0v_QbVJlHNr3Eh3b0oJ8SPEiyzl1bdwqWkojGcwW3cJMGZflkfTqmiBscgNwFbNjIa7cQU01AAjS20HqvcOo2ixQlc3UabFBWOUVDE8oO26jSEZbT385P8BSF5zQ33eox4xxflQ084lrWsYk5ljZytrQ0BuRSm1pbBRas9b_xVh_Fr0RXqviu8Jhk6fb3ya6oytQugRXsq8iWR5WkYhU-7j5o4S_y_ptt5Re6Xbq08Sr_-2csJ5cSVbwqz_R370IrW04ulQaXepy59jmu0hdcwgyd5Fhiuc-c5UVnQZwhbdaOkU4w3_pW8khlY0g9F1ZbfvgaB9xnOkoYcwgYSJ0FI-g4JMZax1O_we_cjnQx52d_XMsjwR_-4c4fBQJFel0fVj1_aXxejkrUjbzy1FUBujo5U4VyW18sQumvWWBsVYoQQir4jZF8B_VmoyqF4FVmsR7B6i59yBdhqZxnjFVhdguY_JVgtfqw0jq-85-uYQFvbZubW9yZnnxu-iegpYhnvaIXe0txUMgXMhk96J33M11iSQnQzMJsRxvdVRVYdIW-UkaR6BaeUj8mW5vUaJjko_v1jnlyc-6tJd_QjJg38ZYgr2mOUqmdjsyI8OjY5ljvsBayJq1r7hj6Mjuwvmp-ZOzShnWcM2dVd1gcM9MziVq3jkwV3oM0BsQpWF9MppQ3MYbyYtylFX1XUhX-8nnO07jX4drkMp6WbVVU2eplyuJIxnyqUQO0ufSJley__lg7pR";

// STT Config
const char* bucketName = "mangdang_voice";
const char* audioContent = "gs://mangdang_voice/audio.wav";
const char* gcAdd = "https://storage.googleapis.com/upload/storage/v1/b/mangdang_voice/o?uploadType=media&name=audio.wav";

// Gemini API Config
const char* apiURL = "https://us-central1-aiplatform.googleapis.com/v1/projects/modern-rex-420404/locations/us-central1/publishers/google/models/gemini-1.5-flash:streamGenerateContent";

//TTS Config
#define I2S_DOUT 17 // Data Out
#define I2S_BCLK 16 // Bit Clock
#define I2S_LRC 15  // Left/Right Clock
Audio audio;
const String baseURL = "https://api.voicerss.org/?key=2ab6d13d5e3d430a91406018dfa44c93&hl=en-us&src=";
const String audioFormat = "&c=mp3&f=16khz_8bit_mono";

HTTPClient _http;

bool isAlphaNumeric(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == ' ');
}

String encodeText(const String& text) {
  String encodedText = "";
  for (int i = 0; i < text.length(); i++) {
    char c = text.charAt(i);
    if (isAlphaNumeric(c)) {
      if (c == ' ') {
        encodedText += "%20";
      } else {
        encodedText += c;
      }
    }
  }
  return encodedText;
}

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

String speechToText() {
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
            return transcript;
        } else {
            Serial.println("Transcript not found.");
        }
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
    }
    _http.end();
    return "";
}

void aitts(String transcript) {
     Serial.println("开始发送 HTTP POST 请求...");
    _http.begin(apiURL);
    _http.addHeader("Content-Type", "application/json");
    _http.addHeader("Authorization", "Bearer " + String(accessToken));

    // 构建 JSON 数据
    String jsonData = "{\"contents\": {\"role\": \"user\", \"parts\": [{\"text\": \"" + transcript + "\"}]}}";
    Serial.print("发送的 JSON 数据: ");
    Serial.println(jsonData);

    int httpResponseCode = _http.POST(jsonData);

    String geminiResponseText = "";
    if (httpResponseCode > 0) {
      String response = _http.getString();
      Serial.print("HTTP 响应代码: ");
      Serial.println(httpResponseCode);
      Serial.print("响应内容: ");
      Serial.println(response);

      int startIndex = 0;
      while (true) {
        startIndex = response.indexOf("\"text\": \"", startIndex);
        if (startIndex == -1) break;
        startIndex += 9;
        int endIndex = response.indexOf("\"", startIndex);
        if (endIndex == -1) break;
        geminiResponseText += response.substring(startIndex, endIndex) + " ";
        startIndex = endIndex + 1;
      }

      Serial.print("组合后的文本: ");
      Serial.println(geminiResponseText);
    } else {
      Serial.print("请求失败，HTTP 响应代码: ");
      Serial.println(httpResponseCode);
    }

    _http.end();

    // TTS 部分
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    //audio.setVolume(21); // 设置音量级别 (0-100)

    // 编码文本并过滤掉非字母数字字符
    String encodedText = encodeText(geminiResponseText);

    // 生成音频 URL
    String audioURL = baseURL + encodedText + audioFormat;
    Serial.println("生成的URL: " + audioURL);

    // 连接并播放音频
    audio.connecttohost(audioURL.c_str());
}