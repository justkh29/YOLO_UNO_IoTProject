#ifndef ESPNOW_RECV_H
#define ESPNOW_RECV_H

#include "global.h"

// ESP-NOW callback
void OnDataRecv(const uint8_t * mac,
                const uint8_t *incomingData,
                int len);

// Tasks
void taskEspNowRecv(void *pvParameters);
void taskRelay(void *pvParameters);

#endif