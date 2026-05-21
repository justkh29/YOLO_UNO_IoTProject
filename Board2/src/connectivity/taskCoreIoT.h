#ifndef TASKCOREIOT_H
#define TASKCOREIOT_H
#include <global.h>

#define MQTT_SERVER "app.coreiot.io"
#define MQTT_PORT 1883
#define DEFAULT_TOKEN "0S8v2psFuSUfZ1DK74KN"

void taskMQTT(void *pvParameters);
#endif // TASKCOREIOT_H