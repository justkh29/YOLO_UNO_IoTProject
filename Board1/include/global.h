#ifndef GLOBAL_H
#define GLOBAL_H

#define MY_SCL 11
#define MY_SDA 12
#define LED1_PIN 18  // GPIO for Device 1
#define LED2_PIN 19  // GPIO for Device 2
#define WIFI_SSID "ESP32_AP1"
#define WIFI_PASS "caimonchoma"
// Global library
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Wire.h>
#include <DHT20.h>
#include <DHTesp.h>
#include <Wifi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h> 
#include <LiquidCrystal_I2C.h> 
#include <Adafruit_NeoPixel.h>
#include <TensorFlowLite_ESP32.h>
#include <PubSubClient.h>
#include <esp_now.h>
#include "../src/device/taskDHT20.h"
#include "../src/device/taskLCD.h"
#include "../src/device/taskNeoPixel.h"
#include "../src/device/taskML.h"
#include "../src/connectivity/taskCoreIoT.h"
#include "../src/connectivity/taskWeb.h"
#include "../src/connectivity/taskEspNowSync.h"
typedef struct {
    SemaphoreHandle_t mutex; // lock for the shared data
    int neo_status;          // status of neopixel
    float temperature;  
    float humidity;
    bool device1;
    bool device2;
} SystemState;

#include <esp_now.h>

// MAC Address của Thiết bị 2 (Relay Node). 
// Bạn cần nạp code rỗng vào thiết bị 2, gọi WiFi.macAddress() để lấy địa chỉ này rồi thay vào đây!
const uint8_t relayNodeMac[] = {0x34, 0x5F, 0x45, 0xA8, 0x65, 0x6C};

// Cấu trúc gói tin gửi qua ESP-NOW
typedef struct struct_message {
    uint8_t magic; // Mã xác nhận chống nhiễu (vd: 0xA5)
    bool relay1_state;
    bool relay2_state;
} struct_message;
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#endif