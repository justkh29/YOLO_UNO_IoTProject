#include "taskCoreIoT.h"
#include <ArduinoJson.h>


static QueueHandle_t rpcQueue = NULL;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) return;

    if (doc.containsKey("device") && doc.containsKey("data")) {
        const char* deviceName = doc["device"];
        JsonObject data = doc["data"];
        const char* method = data["method"];
        
        if (strcmp(deviceName, "relay-device-1") == 0) {
            RpcCommand_t cmd;
            bool isValid = false;

            if (strcmp(method, "setLed1") == 0 || strcmp(method, "setRelay1") == 0) {
                cmd.relayId = 1;
                cmd.state = data["params"];
                isValid = true;
            }
            else if (strcmp(method, "setLed2") == 0 || strcmp(method, "setRelay2") == 0) {
                cmd.relayId = 2;
                cmd.state = data["params"];
                isValid = true;
            }

            if (isValid && rpcQueue != NULL) {
                xQueueSend(rpcQueue, &cmd, 0);
            }
        }
    }
}

bool mqttReconnect() {
    if (client.connected()) return true;

    Serial.print("[MQTT] Connecting to CoreIoT...");
    
    if (client.connect("ESP32_Gateway", DEFAULT_TOKEN, NULL)) {
        Serial.println("OK");
        client.subscribe("v1/gateway/rpc");
        client.publish("v1/gateway/connect", "{\"device\":\"dht20-sensor-1\"}");
        client.publish("v1/gateway/connect", "{\"device\":\"relay-device-1\"}");
        return true;
    }
    Serial.println("Fail");
    return false;
}

void taskMQTT(void *pvParameters) {
    SystemState *sysState = (SystemState *)pvParameters;

    rpcQueue = xQueueCreate(5, sizeof(RpcCommand_t));

    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(callback);

    unsigned long lastMsg = 0;
    const long interval = 5000;

    while (1) {
        if (WiFi.status() == WL_CONNECTED) {
            if (!client.connected()) {
                mqttReconnect();
                vTaskDelay(pdMS_TO_TICKS(5000));
                continue; 
            }
            
            client.loop(); 

            RpcCommand_t cmd;
            while (xQueueReceive(rpcQueue, &cmd, 0) == pdTRUE) {
                if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE) {
                    if (cmd.relayId == 1) sysState->device1 = cmd.state;
                    if (cmd.relayId == 2) sysState->device2 = cmd.state;
                    xSemaphoreGive(sysState->mutex);
                }
                Serial.printf("[MQTT] Đã nhận lệnh RPC -> Relay %d set to %d\n", cmd.relayId, cmd.state);
            }

            unsigned long now = millis();
            if (now - lastMsg > interval) {
                lastMsg = now;

                float t = 0, h = 0;
                bool d1 = false, d2 = false;

                if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE) {
                    t = sysState->temperature;
                    h = sysState->humidity;
                    d1 = sysState->device1;
                    d2 = sysState->device2;
                    xSemaphoreGive(sysState->mutex);
                }

                StaticJsonDocument<512> doc;
                JsonArray dhtArray = doc.createNestedArray("dht20-sensor-1");
                JsonObject dhtData = dhtArray.createNestedObject();
                dhtData["temperature"] = (t > -40) ? t : 0; 
                dhtData["humidity"] = (h > 0) ? h : 0;

                JsonArray relayArray = doc.createNestedArray("relay-device-1");
                JsonObject relayData = relayArray.createNestedObject();
                relayData["led1"] = d1;
                relayData["led2"] = d2;

                char buffer[300];
                serializeJson(doc, buffer);
                client.publish("v1/gateway/telemetry", buffer);
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}