#include "taskML.h"
#include "../model.h"
#include <esp_heap_caps.h> 

#define TIME_STEPS 10
#define NUM_FEATURES 5 // 0:Temp, 1:Humi, 2:hour, 3:day, 4:month

tflite::MicroErrorReporter micro_error_reporter;
tflite::AllOpsResolver resolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;

TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

constexpr int tensorArenaSize = 250 * 1024; 
uint8_t *tensorArena = nullptr; 

float input_buffer[TIME_STEPS][NUM_FEATURES];
int buffer_count = 0;

const float DATA_MIN[NUM_FEATURES] = {24.39, 51.39, 0.0, 6.0, 4.0};
const float DATA_MAX[NUM_FEATURES] = {35.89, 88.68, 23.0, 7.0, 4.0};

float safe_scale(float val, float min_val, float max_val) {
    if (max_val == min_val) return 0.0f; 
    return (val - min_val) / (max_val - min_val);
}

void taskTinyML(void *pvParameters) {
  // Retrieve the global state passed from xTaskCreate
  SystemState *sysState = (SystemState *)pvParameters;

  Serial.println("TinyML Task Starting...");

  tensorArena = (uint8_t *)heap_caps_malloc(tensorArenaSize, MALLOC_CAP_SPIRAM);
  
  if (tensorArena == NULL) {
      Serial.println(" CẢNH BÁO: Không tìm thấy PSRAM hoặc cấu hình sai!");
      Serial.println("-> Tự động chuyển sang dùng RAM nội bộ (Internal SRAM)...");
      
      // Fallback: Cấp phát vào RAM nội bộ của chip
      tensorArena = (uint8_t *)heap_caps_malloc(tensorArenaSize, MALLOC_CAP_INTERNAL);
      
      if (tensorArena == NULL) {
          Serial.println(" LỖI CHÍ MẠNG: RAM nội bộ cũng không đủ! Thử giảm tensorArenaSize xuống.");
          vTaskDelete(NULL);
      } else {
          Serial.printf("Đã cấp phát %d KB thành công trên RAM NỘI BỘ!\n", tensorArenaSize / 1024);
      }
  } else {
      Serial.printf(" Đã cấp phát %d KB thành công trên PSRAM!\n", tensorArenaSize / 1024);
  }

  // 1. Tải mô hình
  tflModel = tflite::GetModel(model_tflite);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    vTaskDelete(NULL);
  }

  // 2. Khởi tạo Interpreter
  interpreter = new tflite::MicroInterpreter(
      tflModel,
      resolver,
      tensorArena,
      tensorArenaSize,
      &micro_error_reporter
  );

  // 3. Chuẩn bị bộ nhớ cho Tensors
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors failed!");
    vTaskDelete(NULL);
  }

  // In ra xem AI thực sự ăn bao nhiêu RAM
  Serial.printf(">> Dung lượng AI thực sự sử dụng: %.2f KB <<\n", interpreter->arena_used_bytes() / 1024.0);

  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("TinyML Ready!");

  while (1) {
    float current_temp = 0.0;
    float current_humi = 0.0;

    // Safely retrieve data from the global state using FreeRTOS mutex
    if (sysState != nullptr && xSemaphoreTake(sysState->mutex, portMAX_DELAY) == pdTRUE) {
        current_temp = sysState->temperature;
        current_humi = sysState->humidity;
        xSemaphoreGive(sysState->mutex);
    }

    // Process only if sensor values are properly populated
    if (current_temp != 0.0f && current_humi != 0.0f) {
        float current_hour  = 12.0; 
        float current_day   = 6.0;  
        float current_month = 4.0;  

        float scaled_temp  = safe_scale(current_temp, DATA_MIN[0], DATA_MAX[0]);
        float scaled_humi  = safe_scale(current_humi, DATA_MIN[1], DATA_MAX[1]);
        
        float scaled_hour  = safe_scale(current_hour,  DATA_MIN[2], DATA_MAX[2]);
        float scaled_day   = safe_scale(current_day,   DATA_MIN[3], DATA_MAX[3]);
        float scaled_month = safe_scale(current_month, DATA_MIN[4], DATA_MAX[4]);

        for(int i = 0; i < TIME_STEPS - 1; i++) {
            for(int j = 0; j < NUM_FEATURES; j++) {
                input_buffer[i][j] = input_buffer[i+1][j];
            }
        }
        
        input_buffer[TIME_STEPS - 1][0] = scaled_temp;
        input_buffer[TIME_STEPS - 1][1] = scaled_humi;
        input_buffer[TIME_STEPS - 1][2] = scaled_hour;
        input_buffer[TIME_STEPS - 1][3] = scaled_day;
        input_buffer[TIME_STEPS - 1][4] = scaled_month;

        if (buffer_count < TIME_STEPS) {
            buffer_count++;
            Serial.printf("Đang thu thập dữ liệu... (%d/%d)\n", buffer_count, TIME_STEPS);
        } else {
            int flat_index = 0;
            for(int i = 0; i < TIME_STEPS; i++) {
                for(int j = 0; j < NUM_FEATURES; j++) {
                    input->data.f[flat_index++] = input_buffer[i][j];
                }
            }

            if (interpreter->Invoke() != kTfLiteOk) {
                Serial.println("Invoke failed!");
            } else {
                float pred_scaled_temp = output->data.f[0];
                float pred_scaled_humi = output->data.f[1];

                float future_temp = (pred_scaled_temp * (DATA_MAX[0] - DATA_MIN[0])) + DATA_MIN[0];
                float future_humi = (pred_scaled_humi * (DATA_MAX[1] - DATA_MIN[1])) + DATA_MIN[1];

                Serial.printf(" Dự báo Nhiệt độ 1H tới: %.2f *C\n", future_temp);
                Serial.printf(" Dự báo Độ ẩm 1H tới:   %.2f %%\n", future_humi);
            }
        }
    }
    
    // Sample periodically (e.g. 5 seconds to build temporal sequence)
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}