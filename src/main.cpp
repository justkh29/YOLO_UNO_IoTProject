#include <global.h>

void setup() {
  
  Serial.begin(115200);
  Wire.begin(11, 12);

  SystemState *sharedData = new SystemState;
  sharedData->blinkDelay = 5000;
  sharedData->mutex = xSemaphoreCreateMutex();
  sharedData->humidity = 0;
  sharedData->temperature = 0;
  // sharedData->device1State = false;
  // sharedData->device2State = false;


  xTaskCreate(taskDHT20, "TempHumid", 4096, (void*)sharedData, 1, NULL);
  xTaskCreate(taskBlinkLED, "Blinking LED", 4096, (void*)sharedData, 1, NULL);
  xTaskCreate(taskNeoPixel, "Neo Pixel", 4096, (void*)sharedData, 1, NULL);
  Preferences preferences;

}

void loop() {
  // Serial.println("Hello Custom Board");
  // delay(1000);
}