#ifndef INC_TASKDHT11_H_
#define INC_TASKDHT11_H_

#include "global.h"

struct SystemState {
    SemaphoreHandle_t mutex; // To protect the shared data
    int ledPin;              // Pin for LED
    int dhtPin;              // Pin for DHT11 Sensor
    int blinkDelay;          // Shared variable: Blink speed in ms
};
void taskDHT11(void *pvParameters);
void taskBlinkLED(void *pvParameters);

#endif 