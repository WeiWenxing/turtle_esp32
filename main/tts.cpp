#include "tts.h"
#include <Arduino.h>
// #ifdef ESP_IDF_VERSION_MAJOR
// #undef ESP_IDF_VERSION_MAJOR
// #endif
// #define ESP_IDF_VERSION_MAJOR 5
#include <Audio.h>

#define I2S_DOUT 17  // Data Out
#define I2S_BCLK 16  // Bit Clock
#define I2S_LRC 15   // Left/Right Clock
const String baseURL = "https://api.voicerss.org/?key=2ab6d13d5e3d430a91406018dfa44c93&hl=en-us&src=";
const String audioFormat = "&c=mp3&f=16khz_8bit_mono";


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

void tts(String text) {
  Audio audio;
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  //audio.setVolume(21); // 设置音量级别 (0-100)

  // 编码文本并过滤掉非字母数字字符
  String encodedText = encodeText(text);

  // 生成音频 URL
  String audioURL = baseURL + encodedText + audioFormat;
  Serial.println("生成的URL: " + audioURL);

  // 连接并播放音频
  audio.connecttohost(audioURL.c_str());

  while (1) {
    audio.loop();
    delay(10);
  }
}
