#include "taskDHT11.h"
void taskDHT11(void *pvParameters)
{
    SystemState *sysState = (SystemState *)pvParameters;
    DHT11 dht(sysState->dhtPin);
    vTaskDelay(pdMS_TO_TICKS(1000));
    while(1)
    {
        int humidity = 0;
        int temperature = 0;
        int res = dht.readTemperatureHumidity(temperature, humidity);
        if (res == 0) // Success
        {
            int newDelay = 1000;
            if (temperature < 20) //Cold TEMP (<20C)
            {
                newDelay = 1000;
            }
            else if (temperature >= 20 && temperature < 30) //NORMAL TEMP [20,30)
            {
                newDelay = 500;
            }
            else //HOT TEMP (>30)
            {
                newDelay = 100;
            }
            if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE)
            {
                sysState->blinkDelay = newDelay;
                xSemaphoreGive(sysState->mutex);
            }
        }
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
}

void taskBlinkLED(void *pvParameters) 
{  
    int current_delay = 1000;
    SystemState *sysState = (SystemState *)pvParameters;
    pinMode(sysState->ledPin, OUTPUT);
    while (1)
    {
        if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE)
        {
            current_delay = sysState->blinkDelay;
            xSemaphoreGive(sysState->mutex);
        }
        digitalWrite(sysState->ledPin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(current_delay));
        digitalWrite(sysState->ledPin, LOW);
        vTaskDelay(pdMS_TO_TICKS(current_delay));
    }
    
}
