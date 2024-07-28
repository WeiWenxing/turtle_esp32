#include "cloud.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include "params.h"

const char* accessToken = "ya29.c.c0ASRK0GaSpIpF0doOQJrKAeqcDoNpwd9WKMoM2l1V4rDx331yAG1XaOWrYd36D0VMXjzACrB9PpgFqbCmmqm7vNeXQdlePNGdYzFYRPkx5-Twr9K8l_04DCpwd2RTxXOUSQ1HpC5LDYiiodtyV4k9cc3qcdeCtAUdWabCPrUZq9YOzTLWuwyYeiRC97U_JX6cYFkvgV68-sJacrl-7WYPkQMqtuLeae8hJR629bDZAFdEbXi3j4uM3JxV0uIq6ukpA1INP0zvXWjxYnIHOrF_6fBNSs0KEtWdaSJH-7kZRAK1o8WQIp1rz-oMX0pUwri3iND2L9mPpfWsrLYKB8lY080Rv87QRJCcy8lXsbQ9akNl70ECP4SpZoIE384Puprdc6lWu0Jg_k9od6znjc1nYzweS-dBhYtudQpzgxtuRZ2tSpt6h_cfz-rqcrWxOUbbml4rUOtpRRlBjrgQpnVI98vrhF0Zo3lqdgqJ9eOmh30J0hj5ej69oqbzfYdJ2gwj3wFykb6yWgVtU61k6fgllkM2ZybJ2diz7OqsalsdksmsYkRqqrvYsvJxdSFoJxbJXbwmFXeh4cre_FZvgh4uUgu9i-rZSgygUaI4dWI3McvVXuW-1UQ9JxBbUrQMzg7-mSciyl27Ypbh_3v9Izrjd6vUwrh1mB5vnQvsp4Ryuhqp9gx1Xsc7fQq60SdW7q210scFBIrwXI097FB_O0ftZ3jihpSyiFvmsSJMSMsIfrn_Owfs7QX1R6qhv5UIvF2c-USvmr3WWjFrFaer1g1S1IveumSm1pvglkWatOVBin-37nm6S1wueQtYnO20B2ja5QSRWJO4Y2xzWBoZxlVv4kkaOq4hlee4x1-ppXYjOk5dYZ4XnzfY992YYYmY8ehQ7IQbB371bjyix7SRc9FqSS07Uyg0tY0-_X2qxvdRUcO7otpf9i6306Bkx4Wcl70JYh0Ug7vf2WWb4mxWOl20p0M5harpocVXr9gfvdSFSjXUttXjlMnzYMe";

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