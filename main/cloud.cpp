#include <Audio.h>

#include "cloud.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include "params.h"

const char* accessToken = "ya29.c.c0ASRK0GZUqFdJL_qn5buJYHWhM9i1rgb3NxT3XMvI4DaI3S4gMoIuUK2pg_RZ3uzejwmTVo1xAZgmAjbuGtHRDaJs8j8gb7hMq9AlxAMsPqgS7cw_QFnblDuyqUOToNvjmVY-62TIuwbCc9B7iG5QFQMOTaNwLFZ2LgwZtUJ4X5ibKsZgs-KrRhSV8wZ6m2O7IKo8AjuYqVsx6TrLm-TZjtNIfAwCRcjvrrfbg5aC2kxzX1rc3hEF16VIxkphtXUGnMDEpt9VbV20jHg8EpJwp8_pq0uz-UftILFw4dWRXujsakj7qDSqwbijROWtIYVtwQgG2S6XFY8NVDY7I1TaFBa5ejYfYnKTp14i4KwDj01af97CO0DTTD0G384CdlQootnY0Fr4oJ-oiB5wr6B8_Yb1ldRvXqmtsjZglytr9rJVqJeJyjQcyp2JStkpjR4zl6BzivZuQ71Xxaz1Z6USBqcbqxlvOdajoq4mcJaskJSv6qdU9Jl_Bpc4FrdslJVyjvrk2I7p6OB16BzpV5kWaiigWeQtV1kndWaSt5X760eFsRb4R7QynJmu3UvMObj32m_Qcbc6df1fgnx8UOJ9tSw6rz2rmI3w6qk7JSeOlsli56ffftQhc63VfbX8v6SQhn3r7nFfcpiS2QIZI574qpk8bW7amdB8QbtVFWbO079kFxJt_k7giucolqrtdMiBcQ_ajV5seyv6iZwrlbqw7OMpQMcUhIib_h8zwfd9Bwp6VX1dgoRgs3nd1vZShp-ezxF095Oz5-_XO-JJZ1e7Q6zUlj5ay3hUtoShbxJfUltw_tBSF2t9z5Jtqk_qIrO6Y858BJteUZXk84gnWU4g-YddlSmYkaJg35gR8x4xzzZW8ck_xb4jgF5uXMQaazqqc6bwO2fupp30-ytho38aJdnow7SwlFhIlWVx6jdvF07sq4e452mlOgYqQZ9jsjFtXnRdp_mpx1i4fs2y172O56B1cgpZIYOiRu42WxXsY3nJM0MVqgkzqhu";

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