#include "taskEspNowRecv.h"

static QueueHandle_t espNowQueue = NULL;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    if (len == sizeof(struct_message)) {
        struct_message receivedData;
        memcpy(&receivedData, incomingData, sizeof(receivedData));
        
        if (receivedData.magic == 0xA5) {
            if (espNowQueue != NULL) {
                xQueueSend(espNowQueue, &receivedData, 0);
            }
        }
    }
}

void taskEspNowRecv(void *pvParameters) {
    SystemState *sysState = (SystemState *)pvParameters;

    espNowQueue = xQueueCreate(10, sizeof(struct_message));

    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    if (esp_now_init() != ESP_OK) {
        Serial.println("[ESP-NOW] Init Failed");
        vTaskDelete(NULL);
    }

    esp_now_register_recv_cb(OnDataRecv);
    Serial.println("[ESP-NOW] Đang lắng nghe lệnh từ Gateway...");

    struct_message msg;
    
    while(1) {
        if (xQueueReceive(espNowQueue, &msg, portMAX_DELAY) == pdTRUE) {
            
            if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE) {
                sysState->device1State = msg.relay1_state;
                sysState->device2State = msg.relay2_state;
                xSemaphoreGive(sysState->mutex);
            }
            
            Serial.printf("[ESP-NOW] Đã cập nhật -> Relay 1: %d | Relay 2: %d\n", 
                          msg.relay1_state, msg.relay2_state);
        }
    }
}