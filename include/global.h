#ifndef GLOBAL_H
#define GLOBAL_H

#define MY_SCL 11
#define MY_SDA 12
// Global library
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Wire.h>
//#include <DHTesp.h>
#include <DHT20.h>
#include "../src/device/taskDHT20.h"
typedef struct {
    SemaphoreHandle_t mutex; // lock for the shared data
    int blinkDelay;          // blink speed in ms
    int neo_status;          // status of neopixel
    int temperature;  
    int humidity;
} SystemState;
#endif