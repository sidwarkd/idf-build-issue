#include "esp32_ble_credential_service.h"
#include <cstdint>
#include <string.h>
#include <algorithm>
#include <esp_event.h>
#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_ble.h>
#include "esp_bt.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include <esp_wifi.h>
#include "esp_log.h"
#include "cJSON.h"

constexpr const char* LOG_TAG = "ble_prov";

static void provisioning_event_handler(void* arg, esp_event_base_t event_base,
                                       int32_t event_id, void* event_data)
{
    ESP32BleWifiCredentialService *service = (ESP32BleWifiCredentialService*)arg;

    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGI(LOG_TAG, "BLE Wifi provisioning started");
                break;
            case WIFI_PROV_CRED_RECV: {
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                ESP_LOGI(LOG_TAG, "Received Wi-Fi credentials"
                         "\n\tSSID     : %s\n\tPassword : %s",
                         (const char *) wifi_sta_cfg->ssid,
                         (const char *) wifi_sta_cfg->password);

                service->ssid = (const char*)wifi_sta_cfg->ssid;
                service->passphrase = (const char*)wifi_sta_cfg->password;
                break;
            }
            case WIFI_PROV_CRED_FAIL: {
                wifi_prov_mgr_reset_sm_state_on_failure();
                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
            {
                ESP_LOGI(LOG_TAG, "BLE Wifi provisioning succeeded!");
                break;
            }
            case WIFI_PROV_END: {
                ESP_LOGI(LOG_TAG, "BLE Wifi provisioning service done");
                wifi_prov_mgr_deinit();
                break;
            }
            case WIFI_PROV_DEINIT: {
                esp_bt_mem_release(ESP_BT_MODE_BTDM);
                break;
            }
            default:
                break;
        }
    }
}

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                          uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{
    char response[128];
    ESP32BleWifiCredentialService *service = (ESP32BleWifiCredentialService*)priv_data;

    ESP_LOGI(LOG_TAG, "Received BLE data: %.*s", inlen, (char *)inbuf);
    //...production code redacted...
    *outbuf = (uint8_t *)strdup(response);
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}

void ESP32BleWifiCredentialService::start()
{
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &provisioning_event_handler, this));
    
    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,
        .app_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    };

    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    ESP_LOGI(LOG_TAG, "Starting provisioning");

    char service_name[12];
    get_device_service_name(service_name, sizeof(service_name));

    wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

    uint8_t custom_service_uuid[] = {
        /* LSB <---------------------------------------
            * ---------------------------------------> MSB */
        0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
        0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
    };
    wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);
    wifi_prov_mgr_endpoint_create("custom-data");

    // TODO: change this from the default. Coordinate with app team
    const char *pop = "abcd1234";
    
    /* Start provisioning service */
    // wifi_prov_mgr_disable_auto_stop(1000);
    ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, pop, service_name, NULL));

    wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, this);
}

void ESP32BleWifiCredentialService::stop()
{
    wifi_prov_mgr_stop_provisioning();
}