#ifndef CLOUD_H
#define CLOUD_H

#include <HTTPClient.h>
#include <SPIFFS.h>


class CloudService {
public:
    CloudService(const char* ssid, const char* password, const char* accessToken, const char* bucketName);
    void uploadFile(const char* filename);
    void receiveFile(const char* audioContent);

private:
    const char* _ssid;
    const char* _password;
    const char* _accessToken;
    const char* _bucketName;
    HTTPClient _http;
};

#endif