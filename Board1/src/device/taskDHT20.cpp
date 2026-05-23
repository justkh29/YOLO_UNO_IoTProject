// #include "taskDHT20.h"
// DHT20 dht;
// void taskDHT20(void *pvParameters)
// {
//     Wire.begin(11, 12);
//     SystemState *sysState = (SystemState *)pvParameters;
//     dht.begin();
//     Serial.println("[DHT] Sensor initialized");
//     vTaskDelay(pdMS_TO_TICKS(1000));
//     while(1)
//     {
//         if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE)
//         {
//             dht.read();
//             sysState->humidity = dht.getHumidity();
//             sysState->temperature = dht.getTemperature();
//             Serial.print("[DHT] Temp: ");
//             Serial.print(sysState->temperature);
//             Serial.print(" C  | Humidity: ");
//             Serial.print(sysState->humidity);
//             Serial.println(" %");            
//             xSemaphoreGive(sysState->mutex);
//         }
//         vTaskDelay(pdMS_TO_TICKS(5000));
//     }
// }
#include "taskDHT20.h"
DHTesp dht;
void taskDHT20(void *pvParameters)
{
    SystemState *sysState = (SystemState *)pvParameters;
    dht.setup(DHTPin, DHTesp::DHT11);
    Serial.println("[DHT] Sensor initialized");
    while(1)
    {
        if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE)
        {
            TempAndHumidity data = dht.getTempAndHumidity();
            if (data.humidity <= 100 && data.humidity >= 0 && data.temperature >= -40 && data.temperature <= 80)
            {
                sysState->humidity = data.humidity;
                sysState->temperature = data.temperature;
                Serial.print("[DHT] Temp: ");
                Serial.print(sysState->temperature);
                Serial.print(" C  | Humidity: ");
                Serial.print(sysState->humidity);
                Serial.println(" %");
            }
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
