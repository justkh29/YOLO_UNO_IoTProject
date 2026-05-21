#include <global.h>

void setup()
{
  Serial.begin(115200);
  Wire.begin(21,22);
  SystemState *sharedData = new SystemState;
  sharedData->mutex = xSemaphoreCreateMutex();
  sharedData->humidity = 0;
  sharedData->temperature = 0;
  sharedData->device1State = false;
  sharedData->device2State = false;

  xTaskCreate(taskDHT20, "TempHumid", 4096, (void *)sharedData, 1, NULL);
  //xTaskCreate(taskBlinkLED, "Blinking LED", 4096, (void *)sharedData, 1, NULL);
  xTaskCreate(taskLCD, "LCD Display", 4096, (void *)sharedData, 1, NULL);
  //xTaskCreate(taskNeoPixel, "Neo Pixel", 4096, (void *)sharedData, 1, NULL);
  xTaskCreate(taskRelay, "Relay Control", 4096, (void *)sharedData, 1, NULL);
  Preferences preferences;
  preferences.begin("wifi-config", true);
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");
  preferences.end();

  if (ssid == "")
  {
    Serial.println("No saved WiFi. Starting AP Mode.");
    xTaskCreate(taskWeb, "Web Server", 8192, (void *)sharedData, 1, NULL);
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

    xTaskCreate(taskWeb, "Web Server", 8192, (void *)sharedData, 1, NULL);
    xTaskCreate(taskMQTT, "MQTT Task", 8192, (void *)sharedData, 1, NULL);
  }
}

void loop()
{
  // Empty
}