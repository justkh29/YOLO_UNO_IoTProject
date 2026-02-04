#include "task1_2.h"
#include "taskDHT11.h"

void task12Initialization(void *pvParameters)
{
    SystemState *sharedData = new SystemState;
    sharedData->ledPin = 48;
    sharedData->dhtPin = 21;
    sharedData->blinkDelay = 1000;
    sharedData->mutex = xSemaphoreCreateMutex();

    xTaskCreate(taskDHT11, "TempHumid", 2048, (void*)sharedData, 1, NULL);
    xTaskCreate(taskBlinkLED, "Blinking LED", 2048, (void*)sharedData, 1, NULL);
    vTaskDelete(NULL);
}