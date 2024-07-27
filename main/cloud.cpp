#include "cloud.h"
#include <WiFi.h>
#include <Arduino.h>
#include "params.h"

const char* accessToken = "ya29.c.c0ASRK0GZu5e9EDKslM_EcXP5bXETOV662il-A3h1At0JQ84CVHLfqR_ZZnMXwyVY-rhSdxHvrOTeEiZqk5v-URB0_fJhIBOEaBE3RObCKSw-NW283bAtjCIcnf7Nb5av3SNwt6r_9AqdLtKBSseIou62Z3WQ_nKiEQxQvnyGCpCrEvX99j5Ly7CwXVs-k8BTa4saeAMUc0JGP5A0qKkEZrbyXQI429OwJ7pEy5QVID6KG__RHViKYQzHCJSpjCN67imU6b3xZ-wWzjiQOhxRd_TL-00FJKylYilYDiNNY-bdXmx8RRxet0WGDUUuH2gCUwRobOLpo8CsouWWc8Ya8nctrLzQY5A_BnrRTdSvjYIsibvYn6a-QQRU8E385Al6l1m2bySwZl4R_Fg65R193jwnOeUYyha-ZymjJv1Yoxrb72-0X48vdbrVVWi37t_U2u4VeJzUkqevOOm78xjpFzyfou12JajjrF9k4_c1MOn_xU1Q28qS5cZd8xsBR48ejWzfbM0sFlJ-a5gXzgbpUV6pI_gmBg140fsbz8W65My_IW43kcc8WWyQgf1cmylmr_kp0mdcr7oirBWeSIk_0x5567sW394pJikZt3Jp4khes75SjVnXe9Xs8n_7mtVqpUucnX4XpIdOjzq8jts9jeXVhMYRz17v7_gnfs4QtVigu7O4z0BMquxmyMosSd0hBqch1W8QMbtMx0bavhiOV7-2OFca7O8vMmesVij-i3w36JMb5O7WwS1ZtXZ-8_-n7dSrr_pebMcwi0qeO_rJXlwkQdhn8t0J1ftWx7o45YzrgW3Yjtiog9p8l3kd0BVj4_Jus8zQZq9fa-iSmF5jfoW9zMF2Vx2JfcIkvOJaSwXdaiue0QjrXOzqRnaM95s9sd_oU_YOvd8UURo26q7lU5Yhdyin7woJRZ1nWVwp1Jjh0Zg3WieX2yZ-_Y8M33bO6F7xIkgmSwk-nBF1U_8VSgvx1Wzw0ym6oduz8h6BhbxWYf1_rcslfvIR";

// STT Config
const char* bucketName = "mangdang_voice";
const char* audioContent = "gs://mangdang_voice/audio.wav";
const char* gcAdd = "https://storage.googleapis.com/upload/storage/v1/b/mangdang_voice/o?uploadType=media&name=audio.wav";

// Gemini API Config
const char* apiURL = "https://us-central1-aiplatform.googleapis.com/v1/projects/modern-rex-420404/locations/us-central1/publishers/google/models/gemini-1.5-flash:streamGenerateContent";

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

String aitts(String transcript) {
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
  return geminiResponseText;
}