#ifndef TASKCOREIOT_H
#define TASKCOREIOT_H
#include <global.h>

#define MQTT_SERVER "app.coreiot.io"
#define MQTT_PORT 1883
#define DEFAULT_TOKEN "RFr39HEfZJxLplkLhx0J"
typedef struct {
    uint8_t relayId; // Lưu số thứ tự Relay (1 hoặc 2)
    bool state;      // Trạng thái ON/OFF
} RpcCommand_t;
void callback(char* topic, byte* payload, unsigned int length);
void taskMQTT(void *pvParameters);
bool mqttReconnect();
#endif // TASKCOREIOT_H