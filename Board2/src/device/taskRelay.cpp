#include "taskRelay.h"
void taskRelay(void *pvParameters)
{
    SystemState *sysState = (SystemState *)pvParameters;
    pinMode(RELAY_1, OUTPUT);
    pinMode(RELAY_2, OUTPUT);
    bool dev1_state = false;
    bool dev2_state = false;
    while(1)
    {
        if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE)
        {
            dev1_state = sysState->device1State;
            dev2_state = sysState->device2State;
            xSemaphoreGive(sysState->mutex);
        }
        digitalWrite(RELAY_1, dev1_state);
        digitalWrite(RELAY_2, dev2_state);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}