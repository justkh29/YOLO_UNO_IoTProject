#ifndef GLOBAL_H
#define GLOBAL_H

#define MY_SCL 11
#define MY_SDA 12
#define RELAY_1 18  // GPIO for Device 1
#define RELAY_2 19  // GPIO for Device 2
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
#include <esp_now.h>
#include "../src/connectivity/taskWeb.h"
#include "../src/device/taskRelay.h"
#include "../src/connectivity/taskEspNowRecv.h"
typedef struct {
    SemaphoreHandle_t mutex; 
    bool device1State; 
    bool device2State;
} SystemState;
const uint8_t relayNodeMac[] = {0x34, 0x5F, 0x45, 0xA8, 0x65, 0x6C};

// Cấu trúc gói tin gửi qua ESP-NOW
typedef struct struct_message {
    uint8_t magic; // Mã xác nhận chống nhiễu 
    bool relay1_state;
    bool relay2_state;
} struct_message;
#endif