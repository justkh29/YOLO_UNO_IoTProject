#include "global.h"

void setup() {
  Serial.begin(115200);

  // Khởi tạo biến toàn cục
  SystemState *sharedData = new SystemState;
  sharedData->mutex = xSemaphoreCreateMutex();
  sharedData->device1State = false;
  sharedData->device2State = false;

  Preferences preferences;
  preferences.begin("wifi-config", true);
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");
  preferences.end();
  xTaskCreate(taskRelay, "Relay control", 2048, (void *)sharedData, 1, NULL);
  xTaskCreate(taskEspNowRecv, "ESP-NOW Recv", 4096, (void *)sharedData, 2, NULL);
  if (ssid == "")
  {
    Serial.println("No saved WiFi. Starting AP Mode.");
    xTaskCreate(taskWeb, "Web Server", 8192, (void *)sharedData, 3, NULL);
  }
  else
  {
    Serial.print("Found saved WiFi: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20)
    {
      delay(500);
      Serial.print(".");
      retries++;
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.print("Connected! IP: ");
      Serial.println(WiFi.localIP());
    }
    else
    {
      Serial.println("Failed to connect. Starting to AP Mode.");
      WiFi.disconnect();
      WiFi.setAutoReconnect(false);
    }

    xTaskCreate(taskWeb, "Web Server", 8192, (void *)sharedData, 3, NULL);
  }
}

void loop() {
  // Xóa task loop mặc định để giải phóng bộ nhớ cho FreeRTOS
  vTaskDelete(NULL);
}
