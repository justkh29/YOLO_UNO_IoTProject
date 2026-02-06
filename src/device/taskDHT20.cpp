#include "taskDHT20.h"
DHT20 dht;
void taskDHT20(void *pvParameters)
{
    Wire.begin(11, 12);
    SystemState *sysState = (SystemState *)pvParameters;
    dht.begin();
    Serial.println("[DHT] Sensor initialized");
    vTaskDelay(pdMS_TO_TICKS(1000));
    int humidity = 0;
    int temperature = 0;
    while(1)
    {
        dht.read();
        humidity = dht.getHumidity();
        temperature = dht.getTemperature();
        Serial.print("[DHT] Temp: ");
        Serial.print(temperature);
        Serial.print(" C  | Humidity: ");
        Serial.print(humidity);
        Serial.println(" %");
        int newDelay = 1000;
        
        if (temperature < 20.0) //Cold TEMP (<20C)
        {
            newDelay = 1000;
        }
        else if (temperature >= 20.0 && temperature < 30.0) //NORMAL TEMP [20,30)
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
