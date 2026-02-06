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

#include "../src/device/task1_2.h"
#include "../src/device/taskDHT20.h"
#endif