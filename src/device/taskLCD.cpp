#include "taskLCD.h"
LiquidCrystal_I2C lcd(33, 16, 2);
void taskLCD(void *pvParameters)
{
    SystemState *sysState = (SystemState *)pvParameters;
    lcd.init();
    lcd.backlight();
    
    float temp = 0;
    float hum = 0;
    bool dev1 = false;
    bool dev2 = false;
    while (1)
    {
        if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE)
        {
            temp = sysState->temperature;
            hum = sysState->humidity;
            dev1 = sysState->device1;
            dev2 = sysState->device2;
            xSemaphoreGive(sysState->mutex);
        }

        // Screen 1: Temperature & Humidity
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("T:"); lcd.print(temp, 0); lcd.print("C H:"); lcd.print(hum, 0); lcd.print("%");
        
        lcd.setCursor(0, 1);
        if (temp < 20  || temp > 30 || hum <= 50 || hum >= 75) lcd.print("Warning");
        else lcd.print("Normal"); 
        
        vTaskDelay(pdMS_TO_TICKS(2500)); 

        //Screen 2: Web Device Status
        lcd.clear();
        if (WiFi.status() == WL_CONNECTED) {
            lcd.setCursor(0, 0);
            lcd.print("WiFi: Connected");
            lcd.setCursor(0, 1);
            lcd.print(WiFi.localIP()); 
        } else {
            lcd.setCursor(0, 0);
            lcd.print("AP Mode: ");
            lcd.print(WIFI_SSID); 
            lcd.setCursor(0, 1);
            lcd.print("IP: 192.168.4.1");
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
        
        // Screen 3: LED and FAN status
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Dev1 (LED): ");
        lcd.print(dev1 ? "ON " : "OFF");

        lcd.setCursor(0, 1);
        lcd.print("Dev2 (FAN): ");
        lcd.print(dev2 ? "ON " : "OFF");

        vTaskDelay(pdMS_TO_TICKS(2500)); 

	}
}
