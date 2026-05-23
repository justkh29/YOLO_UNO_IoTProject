#ifndef ESPNOW_SYNC_H
#define ESPNOW_SYNC_H

#include "global.h"

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

void taskEspNowSync(void *pvParameters);

#endif