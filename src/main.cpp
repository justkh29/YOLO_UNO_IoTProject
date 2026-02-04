#include <global.h>

void setup() {
    Serial.begin(115200);
    xTaskCreate(task12Initialization, "Task 1_2", 2048, NULL, 1, NULL);
}

void loop() {
  // Serial.println("Hello Custom Board");
  // delay(1000);
    vTaskDelete(NULL);

}