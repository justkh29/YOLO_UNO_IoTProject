#include "taskLCD.h"
LiquidCrystal_I2C lcd(33, 16, 2);
void taskLCD(void *pvParameters)
{
    SystemState *sysState = (SystemState *)pvParameters;
    lcd.init();
    lcd.backlight();
    
    float temp = 0;
    float hum = 0;
    while (1)
    {
        if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE)
        {
            temp = sysState->temperature;
            hum = sysState->humidity;
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
	}
}
