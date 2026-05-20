    #include "taskNeoPixel.h"
    Adafruit_NeoPixel NeoPixel1(NEO_NUM, NEO_PIN, NEO_GRB + NEO_KHZ800);

    void taskNeoPixel(void *pvParameters)
    {
        SystemState *sysState = (SystemState *)pvParameters;
        NeoPixel1.begin();
        NeoPixel1.clear();
        NeoPixel1.show();
        while(1)
        {
            if (xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE)
            {
                if (sysState->humidity <= 50.0f)
                {
                    NeoPixel1.setPixelColor(0, NeoPixel1.Color(0, 0, 255));
                }
                else if (sysState->humidity <= 75.0f)
                {
                    NeoPixel1.setPixelColor(0, NeoPixel1.Color(0, 255, 0));
                }
                else if (sysState->humidity <= 100.0f)
                {
                    NeoPixel1.setPixelColor(0, NeoPixel1.Color(255, 0, 0));
                }
                NeoPixel1.show();
                xSemaphoreGive(sysState->mutex);
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }