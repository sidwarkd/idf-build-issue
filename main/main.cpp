#include <stdio.h>
#include <thread>
#include <chrono>
#include "esp32_ble_credential_service.h"

extern "C" void app_main(void)
{

    ESP32BleWifiCredentialService service = ESP32BleWifiCredentialService();
    service.start();
    while(1){
        printf("Hello world!\n");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
