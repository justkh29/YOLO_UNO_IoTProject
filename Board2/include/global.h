#ifndef GLOBAL_H
#define GLOBAL_H

#define MY_SCL 11
#define MY_SDA 12
#define DEVICE1_PIN 18  // GPIO for Device 1
#define DEVICE2_PIN 19  // GPIO for Device 2
#define WIFI_SSID "ESP32_AP2"
#define WIFI_PASS "HCMUT2"
// Global library
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Wire.h>
#include <Wifi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h> 
#include <DHT20.h>
#include <DHTesp.h>
#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include "../src/device/taskDHT20.h"
#include "../src/device/taskLCD.h"
#include "../src/connectivity/taskWeb.h"
#include "../src/connectivity/taskCoreIoT.h"
#include "../src/device/taskRelay.h"
typedef struct {
    SemaphoreHandle_t mutex; // To protect the shared data
    int neo_status; 
    float temperature;
    float humidity;
    bool device1State; 
    bool device2State;
    int aiPrediction;
} SystemState;

#endif