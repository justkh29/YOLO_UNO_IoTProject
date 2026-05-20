#ifndef GLOBAL_H
#define GLOBAL_H

#define MY_SCL 11
#define MY_SDA 12
#define LED1_PIN 18  // GPIO for Device 1
#define LED2_PIN 19  // GPIO for Device 2
// Global library
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Wire.h>
#include <DHT20.h>
#include <LiquidCrystal_I2C.h> 
#include <Adafruit_NeoPixel.h>
#include "../src/device/taskDHT20.h"
#include "../src/device/taskLCD.h"

typedef struct {
    SemaphoreHandle_t mutex; // lock for the shared data
    int neo_status;          // status of neopixel
    float temperature;  
    float humidity;
    bool device1;
    bool device2;
} SystemState;
#endif