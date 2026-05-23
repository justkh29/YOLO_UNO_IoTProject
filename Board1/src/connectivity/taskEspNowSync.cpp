#include <global.h>
#include <esp_now.h>
#include <WiFi.h>


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
}

void taskEspNowSync(void *pvParameters) {
    SystemState *sysState = (SystemState *)pvParameters;

    uint8_t relayNodeMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 
    esp_now_peer_info_t peerInfo = {};
    struct_message sendData;
    sendData.magic = 0xA5;

    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    if (esp_now_init() != ESP_OK) {
        vTaskDelete(NULL);
    }

    esp_now_register_send_cb(OnDataSent);
    
    memcpy(peerInfo.peer_addr, relayNodeMac, 6);
    peerInfo.channel = 0; 
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        vTaskDelete(NULL);
    }

    bool last_relay1 = false;
    bool last_relay2 = false;

    while(1) {
        bool current_relay1 = false;
        bool current_relay2 = false;

        if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE) {
            current_relay1 = sysState->device1;
            current_relay2 = sysState->device2;
            xSemaphoreGive(sysState->mutex);
        }

        if (current_relay1 != last_relay1 || current_relay2 != last_relay2) {
            sendData.relay1_state = current_relay1;
            sendData.relay2_state = current_relay2;
            
            esp_err_t result = esp_now_send(relayNodeMac, (uint8_t *) &sendData, sizeof(sendData));
            
            if (result == ESP_OK) {
                last_relay1 = current_relay1;
                last_relay2 = current_relay2;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}