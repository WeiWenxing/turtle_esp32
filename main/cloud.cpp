#include <Audio.h>

#include "cloud.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include "params.h"

const char* accessToken = "ya29.c.c0ASRK0GYq-SvoER8aOZImdSjp4rUiL6sJrcQjYc3Y4VhIcXL3159oEsZB9dYcQ7qpTxITCfGE0TG66t5dWirMD_9R2q_u1uRj6RO_H6VVOx2eUPpsJEI7irEcDHfvf9TWovJQbzc1fXjSU9DHeAhOGA1B1fAt4O5zIxb-jW0oWZRkSJKQE-ZlOVWjzW1v4FHiQlLvjr_-IqN_MFJypYIMeK44LEeYDQVrd2kpgxp36mHU6Tk64w8afo7yq1R8Ng1wFdp9C0_AZpqQq46DQIFw3J99Axj8JwyxaDP8cpmIAahMe3hr2qtWYtM7PYLuHPf8syFS32Vqems8HBmcjnNQUPT545n8k5vLsJ5zstGfHLQg3gdPmcjOpkGghgE387CzobmiuOyyuierQ0061XtJzfYpXR9xapqlYtpe21mypO3oB53xcs2jIxUrw3ZQg7634s80tcF07S9e5QSazv7_3_mj9avmjl0-mhvW7bflegJ_keQlq3OUyFOcsw_Ba6QfIrlUjxxyusoYUjxkUZ-yUXsSUeiuz5uBXzMl1oWgXYWB904RlUvjtoMk2VfZc7WFgyznjXcfdpB-39jp-XW1YeVvZbslc6geXqcQVs42hiJz1atavXzv5utV9R3fbSrdauxc6M_psJqmXvYrZnwZ99dqSt2FIg8ZUqF48sptRr1MJlcXRdh8vgOZaXoyoow4Ycv4cSxZX2M2Ipjq-3BefUqJBrR9_9jxbtwZ9bkdqMtYb2IdVhcI_uJcbthSFZd-S8iOj4WVI5k_9yIM-4n1X191-puVQ-lJ6ovWohdYVsaYpwaIsRRpimRa69bzn9JV-qX5UxvVV_sbR_O5FzSqk83eWUppzYktVjfcb73JYceaU-gQMSje3Rp78yfS_20xvYqmfQwtrzRie-twcVt88vIhWrZMxSvUaQ3UBaiY_p39oaYWOXz2cew84yhB9cJe1_62Z7RU4npuwUwXW3ozjn3SVOJ_i-5n_oJ36Rw2wYkzl2q1VmZvu3z";

// STT Config
const char* bucketName = "mangdang_voice";
const char* audioContent = "gs://mangdang_voice/audio.wav";
const char* gcAdd = "https://storage.googleapis.com/upload/storage/v1/b/mangdang_voice/o?uploadType=media&name=audio.wav";

// Gemini API Config
const char* apiURL = "https://us-central1-aiplatform.googleapis.com/v1/projects/modern-rex-420404/locations/us-central1/publishers/google/models/gemini-1.5-flash:streamGenerateContent?alt=sse";

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

String generateJsonString(String text) {
    return "{"
        "\"contents\": ["
            "{"
            "\"role\": \"USER\","
            "\"parts\": { \"text\": \"Now, you are a small female robo turtle, your name is Amy. You will be a helpful AI assistent. Your LLM api is connected to STT and TTS models so you are able to hear the user.\" }"
            "},"
            "{"
            "\"role\": \"MODEL\","
            "\"parts\": { \"text\": \"OK, I know.\" }"
            "},"
            "{"
            "\"role\": \"USER\","
            "\"parts\": { \"text\": \"" + text + "\" }"
            "}"
        "]"
        ",\"safety_settings\": {"
            "\"category\": \"HARM_CATEGORY_SEXUALLY_EXPLICIT\","
            "\"threshold\": \"BLOCK_LOW_AND_ABOVE\""
        "},"
        "\"generation_config\": {"
            "\"temperature\": 0.2,"
            "\"topP\": 0.8,"
            "\"topK\": 40,"
            "\"maxOutputTokens\": 200"
        "}"
        "}";
}


String llm_response(String transcript) {
  Serial.println("开始发送 HTTP POST 请求...");
  _http.begin(apiURL);
  _http.addHeader("Content-Type", "application/json");
  _http.addHeader("Authorization", "Bearer " + String(accessToken));

  // 构建 JSON 数据
  // String jsonData = "{\"contents\": {\"role\": \"user\", \"parts\": [{\"text\": \"" + transcript + "\"}]}}";
  String jsonData = generateJsonString(transcript);
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
  return geminiResponseText;
}