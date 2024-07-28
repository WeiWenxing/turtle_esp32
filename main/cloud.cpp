#include "cloud.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include "params.h"

const char* accessToken = "ya29.c.c0ASRK0GbvtIwvCzEeowuVsb_pKdLB2idQac5RE_Y4sgHFZgQD534jUPgYm1DVEWrZSS7Tn5gEmv1q7AN5q0-RLQVGR8CmjRUKXxVi2af4GAHuWXzZ9uHed5g6zGP8Jf_8EztuvADcxh43jgLjVWRGdSCX0FBsj7lCsRpjsHhaU2A9M0BM9EW70GoCOQV_ZapP_abkrXUgRL6k7uzKWSmXKDdJThPFHRVLehxkZRgvmO8NdffH42itrBDCT-S7inNxsSZnYiLdbLx8AC1xG29ATwM_nYNhvpFTO4eDENO60iKXWSvLX4WrREa_ApDrrHweZDJ4BGpyFSvX8AXZ1Y2AepRv4u0h95Zs85UU4gTL9ksXhIb63hG6w94H384K7Uyt3oztcIUM6dq1aO_oMWnMwFms5RQuag62SJzWjUpeV6cfF8-Fjq76XgI-dj6a0v6iFuZW53cmRlmqs-v5bYMfRYYbnII3ZXqmgwfhcOwhBUIn0b8wSBMMMJqF_XQtncig-Z9V0J_dvfM-ZumccVbxaR6_qg20Y7fljrrza1zw3Wye_ackqJ3y5pMMM-U9rxU9iMym3U4p9e9id04X7FeJxjXrBS5M70n8dY69yqSXF_wnrWjhk0i7WtnvB9MaS5mpUQ2FlXvSkx2d4Me2_cyazZjpg9898OUeORj9BsqRBbm7u1ZJ7JZz9iI0Q-gRWS22wc0lFgrRS-FhVcZkl8QsxQsj2FbmSB5Vfrv_wF9b7w7rjzV7g6wbsiqIMtYIu97lug6mUaqYO3VkcY5pf6z2h2MXhhwtcFuQWMbyjor81m2Yr9_r7IUVopo93abOmYVf8JSvjwWMBpuaI7o4y1Xcry8cYtq3Z4YbldWdtR8fnXcXf-dccuXqzvcQMqY0pa74UtQt0W4vwxkmFQOWVjyQl1S0OJe6I8_BvdFngt8i9vpnbk76dwM-j725R2XsteX-r7JkVWicR8g3q_7ZxyW1Brr1_Rhs4sBagcFIjQQr4exuBBdhWWRgxiV";

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