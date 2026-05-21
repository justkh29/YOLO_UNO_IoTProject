#include "taskRelay.h"
void taskRelay(void *pvParameters)
{
    SystemState *sysState = (SystemState *)pvParameters;
    pinMode(DEVICE1_PIN, OUTPUT);
    pinMode(DEVICE2_PIN, OUTPUT);
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
        digitalWrite(DEVICE1_PIN, dev1_state);
        digitalWrite(DEVICE2_PIN, dev2_state);
    }
}