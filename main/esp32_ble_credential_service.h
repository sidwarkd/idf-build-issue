#ifndef ESP32_BLE_CREDENTIAL_SERVICE_H
#define ESP32_BLE_CREDENTIAL_SERVICE_H

#include <string>

class ESP32BleWifiCredentialService
{
public:
    void start();

    std::string ssid;
    std::string passphrase;

private:
    void stop();
};

#endif //ESP32_BLE_CREDENTIAL_SERVICE_H