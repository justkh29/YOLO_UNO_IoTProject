#include "taskDHT20.h"
DHT20 dht;
void taskDHT20(void *pvParameters)
{
    //Wire.begin(11, 12);
    SystemState *sysState = (SystemState *)pvParameters;
    dht.begin();
    Serial.println("[DHT] Sensor initialized");
    vTaskDelay(pdMS_TO_TICKS(1000));
    int humidity = 0;
    int temperature = 0;
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
            int newDelay = 1000;
            
            if (sysState->temperature < 20.0) //Cold TEMP (<20C)
            {
                newDelay = 1000;
            }
            else if (sysState->temperature >= 20.0 && sysState->temperature < 30.0) //NORMAL TEMP [20,30)
            {
                newDelay = 500;
            }
            else //HOT TEMP (>30)
            {
                newDelay = 100;
            }
            sysState->blinkDelay = newDelay;
            xSemaphoreGive(sysState->mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void taskBlinkLED(void *pvParameters) 
{  
    int current_delay = 1000;
    int newDelay = 1000;
    SystemState *sysState = (SystemState *)pvParameters;
    pinMode(48, OUTPUT);
    while (1)
    {
        if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE)
        {
            if (sysState->temperature < 20.0) //Cold TEMP (<20C)
            {
                newDelay = 1000;
            }
            else if (sysState->temperature >= 20.0 && sysState->temperature < 30.0) //NORMAL TEMP [20,30)
            {
                newDelay = 500;
            }
            else //HOT TEMP (>30)
            {
                newDelay = 100;
            }
            current_delay = newDelay;
            xSemaphoreGive(sysState->mutex);
        }
        digitalWrite(48, HIGH);
        vTaskDelay(pdMS_TO_TICKS(current_delay));
        digitalWrite(48, LOW);
        vTaskDelay(pdMS_TO_TICKS(current_delay));
    }
    
}