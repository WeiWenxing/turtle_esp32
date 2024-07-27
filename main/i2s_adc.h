#ifndef I2S_ADC_H
#define I2S_ADC_H

#include <Arduino.h>
#include <driver/i2s.h>
#include <SPIFFS.h>
#include <Task.h>

#define I2S_WS 10
#define I2S_SD 4
#define I2S_SCK 11
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE   (16000)
#define I2S_SAMPLE_BITS   (16)
#define I2S_READ_LEN      (16 * 1024)
#define RECORD_TIME       (10)
#define I2S_CHANNEL_NUM   (1)
#define FLASH_RECORD_SIZE (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORD_TIME)

extern File file;

void record_init();
void record();
void delete_rec_file();

#endif
