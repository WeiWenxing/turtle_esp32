#include <Audio.h>

#include <WiFi.h>
#include "Audio.h"

const char* ssid = "Mangdang";
const char* password = "mangdang";

#define I2S_DOUT 17  // Data Out
#define I2S_BCLK 16  // Bit Clock
#define I2S_LRC 15   // Left/Right Clock

Audio audio;
const String baseURL = "https://translate.google.com/translate_tts?ie=UTF-8&client=tw-ob&tl=en&q=";
// String audioFormat = "&c=mp3&f=16khz_8bit_mono";


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

int countWords(const String& text) {
  int count = 1;
  for (int i = 0; i < text.length(); i++) {
    if (text.charAt(i) == '+' || text.charAt(i) == ' ' || text.charAt(i) == '%') {
      count++;
    }
  }
  return count;
}

String removeNewlines(String text) {
  int index = 0;
  index = text.indexOf('\\n');
  Serial.println(index);
  while ((index = text.indexOf('\\n')) != -1) {
    text = text.substring(0, index) + text.substring(index + 1);
  }
  return text;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  //audio.setVolume(21); // Set volume level (0-100)
  audio.setFileLoop(false);

  Serial.println("请输入要转换成语音的文字，然后按回车键：");

  int internal = 5000;
  unsigned long split_time = millis();
  int file_duration, cur_audio_time;
  while (1) {
    if (Serial.available() > 0) {
      String text = Serial.readStringUntil('\n');
      text.trim();  // 移除输入字符串两端的空格和换行符

      if (text.length() > 0) {
        audio.stopSong();
        // text = removeNewlines(text);
        text.replace("\\n", "");
        Serial.print("final text: ");
        Serial.println(text);
        String encodedText = encodeText(text);
        Serial.print("encodedText: ");
        Serial.println(encodedText);

        // String audioURL = baseURL + encodedText;
        String audioURL = text;
        Serial.println("生成的URL: " + audioURL);
        audio.connecttohost(audioURL.c_str());
        file_duration = audio.getAudioFileDuration();
        Serial.print("File Duration: ");
        Serial.println(file_duration);
        split_time = millis();
        internal = 400 * countWords(text);
        Serial.print("internal: ");
        Serial.println(internal);
      }
    }

    audio.loop();  // 必须反复调用以保持音频播放
    delay(1);

    unsigned long curtime = millis();
    if (curtime - split_time > internal) {
      split_time = curtime;
      // cur_audio_time = audio.getAudioCurrentTime();
      // Serial.print("cur_audio_time: ");
      // Serial.println(cur_audio_time);
      audio.stopSong();
    }
  }
}

void loop() {
}

void audio_id3data(const char* info) {  // 当找到ID3数据时调用
  Serial.print("id3data: ");
  Serial.println(info);
}

void audio_eof_mp3(const char* info) {  // 当MP3文件播放结束时调用
  Serial.print("eof_mp3: ");
  Serial.println(info);
}

void audio_eof_speech(const char* info) {  // 当speech播放结束时调用
  Serial.print("audio_eof_speech: ");
  Serial.println(info);
}

void audio_eof_stream(const char* info) {  // 当stream播放结束时调用
  Serial.print("audio_eof_stream: ");
  Serial.println(info);
}
