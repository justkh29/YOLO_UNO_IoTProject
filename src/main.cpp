#include <global.h>

void setup() {
  
  Serial.begin(115200);
  SystemState *sharedData = new SystemState;
  sharedData->ledPin = GPIO_NUM_48;
  sharedData->blinkDelay = 5000;
  sharedData->mutex = xSemaphoreCreateMutex();
  xTaskCreate(taskDHT20, "TempHumid", 4096, (void*)sharedData, 1, NULL);
  xTaskCreate(taskBlinkLED, "Blinking LED", 4096, (void*)sharedData, 1, NULL);

}

void loop() {
  Serial.println("Hello Custom Board");
  delay(1000);
}