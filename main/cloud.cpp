#include <Audio.h>

#include "cloud.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include "params.h"

const char* accessToken = "ya29.c.c0ASRK0GZDcx6Gc3B24SjUVb1Zw6R1F7cU8jN5W-EaNmZFG3Q2YsKeSu3iI7wgFeEqFP3uNGjUY5GefbY_rZE-iBxqPHfjfTcoRyRIRLpH0f6jpGqThw4_47Zyitp7eFkJA3MwhS25Gb8jkfPnJvgvszFccVlj5rywRFsC9cNz-NEqU9rTi1AdoqIgrHqBJtyVUKLnFHhZDEgX21a9G_jaHvioA2m0fkcPYh6C_Ov9SlRwXqIPpmTcEMHoJgyQKFQtqjQs1TGUCr2Lu2UlsFL1-GqeEz6PLwNwFW-XfUtLAHbNhxuNedGkb7EQZJaEFuez8KuCOhncuqTSBmlAVEH_ejCeqXBiZNhDy5SvKSM1sMd7jCp-QSFOkigT384KYb0sttagBu6vBr4Rh279JJ84frW94e0M-RV2iV3u26OWnpk7zxi1Xmgp-3VvmgFxj5OQfQ5XYbtOs78voSh4mlnm50zsBml1iVcpa9_8rh2dVuzyO9B0lk8_Jb1FlnxylM73jzZ1y25w-V07xeRBlk8B1397QZkbFoMhJqqqkRWs2jywsdtxcgsp-5ZUkkRpFwqmRjQ8uOiSiJlz7RpFyXvQ_isxQ8BUbh-g2sryVJxZuvlQaon-dwB1YaJkMu3hzxJeo44Jugop2htaOnlJS-i4r_a2YRks85ag3gY6eUtZ8j4Zx7_Vkk8-buzZVoj4Wg6ud1kchnVs9znB3dm86Ujkgu-9Z-_zMd7Fv_9zaXiZIXa1lbbipf6Sjp72nxvJ4FwbszrcceY9rbwIqoXRwYsUnU29wWyM7dV7aQ-e1q9WJ65JwS6SyIBOXSU7rIM-uhJkI7Mbm_3XQOM_swIiJoa8gcq1if9V7Wenwxaupb5M3pOVuO1JVIBaUXRic7RI8R2J5o5BkmXcyQoUbSYRUlnJz0RxJvVk6em9e6svSp3M2Q7ZSkb-gSl3ws41F6WO3ukrRUicsFMaoWOX_epoiw5U12MfFOslmSMb5M8JuttMhV-sOgJe9IaaBQB";

// STT Config
const char* bucketName = "mangdang_voice";
const char* audioContent = "gs://mangdang_voice/audio.wav";
const char* gcAdd = "https://storage.googleapis.com/upload/storage/v1/b/mangdang_voice/o?uploadType=media&name=audio.wav";

// Gemini API Config
const char* apiURL = "https://us-central1-aiplatform.googleapis.com/v1/projects/modern-rex-420404/locations/us-central1/publishers/google/models/gemini-1.5-flash:streamGenerateContent?alt=sse";


void uploadFile() {
  Serial.println("Opening file...");
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    Serial.println("Failed to open file");
    file.close();
    return;
  }
  Serial.println("File opened successfully");

  HTTPClient _http;

  // String url = "https://storage.googleapis.com/upload/storage/v1/b/" + String(_bucketName) + "/o?uploadType=media&name=audio.wav";
  Serial.print("Connecting to URL: ");
  Serial.println(gcAdd);
  _http.begin(gcAdd);
  _http.addHeader("Authorization", "Bearer " + String(accessToken));
  _http.addHeader("Content-Type", "application/octet-stream");

  Serial.println("Sending POST request...");
  Serial.println(file.size());
  int httpCode = _http.sendRequest("POST", &file, file.size());
  Serial.println("POST request sent");

  if (httpCode > 0) {
    String response = _http.getString();
    // Serial.println("Response: " + response);
  } else {
    Serial.println("Error on file upload, HTTP code: " + String(httpCode));
  }

  file.close();
  _http.end();
  Serial.println("File closed and HTTP connection ended");
}

const  String speechRequestData = "{\"config\": {\"encoding\":\"LINEAR16\",\"languageCode\":\"en-US\",\"enableWordTimeOffsets\":false},\"audio\":{\"uri\":\"gs://mangdang_voice/audio.wav\"}}";
String speechToText() {
  Serial.println("Speech to text start.");
  HTTPClient _http;

  _http.begin("https://speech.googleapis.com/v1/speech:recognize");
  _http.addHeader("Content-Type", "application/json");
  _http.addHeader("Authorization", "Bearer " + String(accessToken));
  _http.addHeader("User-Agent", "PostmanRuntime/7.40.0");
  _http.addHeader("Accept", "*/*");
  _http.addHeader("Accept-Encoding", "gzip, deflate, br");
  _http.addHeader("Connection", "keep-alive");
  _http.addHeader("x-goog-user-project", "modern-rex-420404");

  Serial.println("Speech request.");
  Serial.println("Speech request post start.");
  int httpResponseCode = _http.POST(speechRequestData);

  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode == 200) {
    String response = _http.getString();
    // Serial.println("Response: ");
    // Serial.println(response);

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
         "\"parts\": { \"text\": \""
         + text + "\" }"
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
  HTTPClient _http;

  Serial.println("开始发送 HTTP POST 请求...");
  _http.begin(apiURL);
  _http.addHeader("Content-Type", "application/json");
  _http.addHeader("Authorization", "Bearer " + String(accessToken));

  // 构建 JSON 数据
  // String jsonData = "{\"contents\": {\"role\": \"user\", \"parts\": [{\"text\": \"" + transcript + "\"}]}}";
  String jsonData = generateJsonString(transcript);
  // Serial.print("发送的 JSON 数据: ");
  // Serial.println(jsonData);

  int httpResponseCode = _http.POST(jsonData);

  String geminiResponseText = "";
  if (httpResponseCode > 0) {
    String response = _http.getString();
    // Serial.print("HTTP 响应代码: ");
    // Serial.println(httpResponseCode);
    // Serial.print("响应内容: ");
    // Serial.println(response);

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