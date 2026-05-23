#include "taskCoreIoT.h"
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient client(espClient);
Preferences mqttPrefs;
SystemState *mqttSysState;

char mqttToken[64];
int mqttFailCount = 0;
const int MQTT_MAX_FAIL = 5;

// --- Load Token Logic ---
void loadMqttToken() {
    mqttPrefs.begin("mqtt", true);
    String token = mqttPrefs.getString("token", "");
    mqttPrefs.end();

    if (token.length() > 0) {
        token.toCharArray(mqttToken, sizeof(mqttToken));
        Serial.print("[MQTT] Loaded saved token: ");
        Serial.println(mqttToken);
    } else {
        strcpy(mqttToken, DEFAULT_TOKEN);
        Serial.print("[MQTT] Using default token: ");
        Serial.println(mqttToken);
    }
}

// --- NEW: Callback for LED Control & Config ---
void callback(char* topic, byte* payload, unsigned int length) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';

    Serial.print("[MQTT] RPC Received: ");
    Serial.println(message);

    // Parse JSON
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.print("[MQTT] JSON Error: ");
        Serial.println(error.c_str());
        return;
    }

    // 1. Get the Method Name
    const char* method = doc["method"];
    
    if (!method) return;

    // --- CASE A: Control LED 1 ---
    if (strcmp(method, "setLed1") == 0) {
        bool newState = doc["params"]; // Get true/false
        
        // Update Shared Memory safely
        if (xSemaphoreTake(mqttSysState->mutex, portMAX_DELAY) == pdTRUE) {
            mqttSysState->device1State = newState;
            xSemaphoreGive(mqttSysState->mutex);
        }
        Serial.print("[MQTT] LED 1 set to: ");
        Serial.println(newState ? "ON" : "OFF");
    }

    // --- CASE B: Control LED 2 ---
    else if (strcmp(method, "setLed2") == 0) {
        bool newState = doc["params"];
        
        if (xSemaphoreTake(mqttSysState->mutex, portMAX_DELAY) == pdTRUE) {
            mqttSysState->device2State = newState;
            xSemaphoreGive(mqttSysState->mutex);
        }
        Serial.print("[MQTT] LED 2 set to: ");
        Serial.println(newState ? "ON" : "OFF");
    }

    // --- CASE C: Update Token (Friend's Logic) ---
    else if (strcmp(method, "updatemqtt") == 0) {
        if (doc.containsKey("params")) {
            const char* newToken = doc["params"];
            Serial.print("[MQTT] Updating Token to: ");
            Serial.println(newToken);

            mqttPrefs.begin("mqtt", false);
            mqttPrefs.putString("token", newToken);
            mqttPrefs.end();
            ESP.restart();
        }
    }
}

// --- Reconnect Logic ---
bool mqttReconnect() {
    if (client.connected()) return true;

    Serial.print("[MQTT] Connecting...");
    
    // Connect with Token
    if (client.connect("ESP32_Client", mqttToken, NULL)) {
        Serial.println("OK");
        mqttFailCount = 0;
        
        // IMPORTANT: Subscribe to RPC topic to receive commands
        client.subscribe("v1/devices/me/rpc/request/+");
        
        return true;
    }

    mqttFailCount++;
    Serial.print("Fail, rc=");
    Serial.println(client.state());
    return false;
}

// --- Main Task ---
void taskMQTT(void *pvParameters) {
    mqttSysState = (SystemState *)pvParameters;

    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(callback); // Register the callback
    loadMqttToken();

    unsigned long lastMsg = 0;
    const long interval = 5000;

    while (1) {
        if (WiFi.status() == WL_CONNECTED) {
            
            // Check Connection
            if (!client.connected()) {
                if (!mqttReconnect()) {
                    // Fail Safe Logic
                    if (mqttFailCount >= MQTT_MAX_FAIL) {
                        Serial.println("[MQTT] Resetting to DEFAULT token.");
                        mqttPrefs.begin("mqtt", false);
                        mqttPrefs.remove("token");
                        mqttPrefs.end();
                        strcpy(mqttToken, DEFAULT_TOKEN);
                        mqttFailCount = 0;
                    }
                    vTaskDelay(pdMS_TO_TICKS(5000));
                    continue; 
                }
            }
            
            client.loop(); // Handle incoming messages (The Callback runs here)

            // Send Telemetry
            unsigned long now = millis();
            if (now - lastMsg > interval) {
                lastMsg = now;

                float t = 0;
                float h = 0;
                bool d1 = false, d2 = false;

                if (xSemaphoreTake(mqttSysState->mutex, portMAX_DELAY) == pdTRUE) {
                    t = mqttSysState->temperature;
                    h = mqttSysState->humidity;
                    d1 = mqttSysState->device1State;
                    d2 = mqttSysState->device2State;
                    xSemaphoreGive(mqttSysState->mutex);
                }

                StaticJsonDocument<200> doc;
                doc["temperature"] = t;
                doc["humidity"] = h;
                doc["led1"] = d1;
                doc["led2"] = d2;

                char buffer[256];
                serializeJson(doc, buffer);
                client.publish("v1/devices/me/telemetry", buffer);
            }
        }
        else {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}