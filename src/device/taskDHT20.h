#ifndef INC_TASKDHT11_H_
#define INC_TASKDHT11_H_

#include "global.h"

typedef struct {
    SemaphoreHandle_t mutex; // To protect the shared data
    int ledPin;              // Pin for LED
    int blinkDelay;          // Shared variable: Blink speed in ms
} SystemState;
extern void taskDHT20(void *pvParameters);
extern void taskBlinkLED(void *pvParameters);

#endif 