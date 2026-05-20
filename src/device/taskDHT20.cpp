#include "taskDHT20.h"
DHT20 dht;
void taskDHT20(void *pvParameters)
{
    Wire.begin(11, 12);
    SystemState *sysState = (SystemState *)pvParameters;
    dht.begin();
    Serial.println("[DHT] Sensor initialized");
    vTaskDelay(pdMS_TO_TICKS(1000));
    float humidity = 0;
    float temperature = 0;
    while(1)
    {
        if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE)
        {
            dht.read();
            sysState->humidity = dht.getHumidity();
            sysState->temperature = dht.getTemperature();
            Serial.print("[DHT] Temp: ");
            Serial.print(sysState->temperature);
            Serial.print(" C  | Humidity: ");
            Serial.print(sysState->humidity);
            Serial.println(" %");            
            xSemaphoreGive(sysState->mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void taskBlinkLED(void *pvParameters) 
{  
    int current_delay = 1000;
    SystemState *sysState = (SystemState *)pvParameters;
    pinMode(48, OUTPUT);
    while (1)
    {
        if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE)
        {
            if (sysState->temperature < 20.0) //Cold TEMP (<20C)
            {
                current_delay = 1000;
            }
            else if (sysState->temperature >= 20.0 && sysState->temperature < 30.0) //NORMAL TEMP [20,30)
            {
                current_delay = 500;
            }
            else //HOT TEMP (>30)
            {
                current_delay = 100;
            }
            xSemaphoreGive(sysState->mutex);
        }
        digitalWrite(48, HIGH);
        vTaskDelay(pdMS_TO_TICKS(current_delay));
        digitalWrite(48, LOW);
        vTaskDelay(pdMS_TO_TICKS(current_delay));
    }
    
}
