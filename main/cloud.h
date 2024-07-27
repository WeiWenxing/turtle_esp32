#ifndef CLOUD_H
#define CLOUD_H

#include <HTTPClient.h>
#include <SPIFFS.h>
#include "params.h"

void uploadFile();
String speechToText();
void aitts(String transcript);

#endif