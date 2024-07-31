#include <Audio.h>

#include "cloud.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include "params.h"

const char* accessToken = "ya29.c.c0ASRK0GZ9QSkFs91Ea-Bgk1EDxs5v7WLzVLEpPvorVtWtyPDWFhUOnKA5SiecHZYefwiSEXfjxQGTrgGkZnaBO6UzTEHXgb-AYqYfAziyivA17UuOFTVcz19Gydh4_-PP8XQyHBhqAdmc0t9J4jDomYv04Hv-1-XLyAcBIjQPr1BAitcvVfLeTq-9lqPqHXhp4LxI6MiFOyRSCS0V5fs2fy43JDcVJ5wxVTFqY4sIi0JvLS8weCZvGKcn9Bv1_oHgI6q3N-XaY-XPoVEwKFztG1VPk_2pUPMhHFOF6H05Miq_D1AJCMd07Sh50rCCXYvcAlchYa8_at7fqY4J7h07Qv8WOG7m-RjD4TCjHbis6Z7YJNJLEpyA97_R2AT387PonzMkpRJ2w5VI_njrzMB9Xb8BIbYnzO3JhxnMOZYihzhV-R_2mtwWQWy_ldyhB48ltUWaBihJRXutrMaMn-QFScQOczqwO6OicBo7fyM2_v7xXhhu3Ofkk_yrQWqplSObYff11uquxgSn3JwQJiasd6w_fpBlVocuhfjikvWqjufFxSU0mSyiMs3ht45F25crfo1xndmWo27_-mOBwx0fssSF-_9_d1QIaaXZb7zvkfhyucZOWZdxFqZcknztWezqZaQrO9ywyX4fI81d6wQROx6vF5F56ftJsk8xqd9sw5z0inhcqWy9QVgv5zXhotUns6i0jcxR8VbyXhdmOh_51vkv84f-wMXstwh-ZxM1i2XsXUw9zw_imI5v9d29n0-rySuFIQWMfn55J91ngaq_yX3R3clsenv0BsO2myX-r3asbWt0-RB8f8VYF6XqurQiw0tOhbR0naBS26J6_9UdjW-m02R3MbFyhfalafuB9eS9-9Z256QvoUjpQhSgak2mj5cjIze1v8sIcX5oZ7sutobXFe-J2l8O5Fyu5bSs5iw1a5W_iJteVROQSrwtunr9Svoqi02-d82cqc4ekZ5-hqn0tWwFt4MwcQSmWibM-IoSuORaJ5Q-gn7";

// STT Config
const char* bucketName = "mangdang_voice";
const char* audioContent = "gs://mangdang_voice/audio.wav";
const char* gcAdd = "https://storage.googleapis.com/upload/storage/v1/b/mangdang_voice/o?uploadType=media&name=test2.wav";

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
            "\"maxOutputTokens\": 50"
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