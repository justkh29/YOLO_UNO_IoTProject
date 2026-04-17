#include <global.h>

void setup() {
  
  Serial.begin(115200);
  SystemState *sharedData = new SystemState;
  sharedData->mutex = xSemaphoreCreateMutex();
  sharedData->humidity = 0.0;
  sharedData->temperature = 0.0;
  sharedData->neo_status = 0;
  xTaskCreate(taskDHT20, "TempHumid", 4096, (void*)sharedData, 1, NULL);
  xTaskCreate(taskLCD, "LCD Display", 4096, (void*)sharedData, 1, NULL);

}

void loop() {
}