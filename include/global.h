#ifndef GLOBAL_H
#define GLOBAL_H

#define MY_SCL 11
#define MY_SDA 12
#define LED1_PIN 18  // GPIO for Device 1
#define LED2_PIN 19  // GPIO for Device 2
#define WIFI_SSID "ESP32_AP"
#define WIFI_PASS "caimonchoma"
// Global library
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Wire.h>
#include <Preferences.h>
#include <ArduinoJson.h> 
#include <DHT20.h>
#include <DHTesp.h>
#include <Adafruit_NeoPixel.h>
#include <PubSubClient.h>
#include "../src/device/taskDHT20.h"
#include "../src/device/taskNeoPixel.h"
typedef struct {
    SemaphoreHandle_t mutex; // To protect the shared data
    int blinkDelay;          // Shared variable: Blink speed in ms
    int neo_status; 
    int temperature;
    int humidity;
    bool device1State; 
    bool device2State;
} SystemState;
#endif