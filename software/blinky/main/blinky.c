#include <stdio.h>
#include <stdbool.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    ESP_LOGI("main", "Hello from main");
    
    while(true) {
	vTaskDelay(120 / portTICK_PERIOD_MS);
    }
}
