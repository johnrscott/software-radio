#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"

#define RMT_LO_I_GPIO_NUM 8 // In-phase local oscillator output
#define RMT_LO_Q_GPIO_NUM 8 // Quadrature local oscillator output

#define DSHOT_ESC_RESOLUTION_HZ 40000000 // 40MHz resolution, DSHot protocol needs a relative high resolution


static const char *TAG = "sdr";


void app_main(void)
{
    ESP_LOGI(TAG, "Started program");

    ESP_LOGI(TAG, "Create RMT TX channel");
    rmt_channel_handle_t esc_chan = NULL;
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select a clock that can provide needed resolution
        .gpio_num = RMT_LO_I_GPIO_NUM,
        .mem_block_symbols = 64,
        .resolution_hz = DSHOT_ESC_RESOLUTION_HZ,
        .trans_queue_depth = 10, // set the number of transactions that can be pending in the background
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &esc_chan));

    
    while (1) {
	vTaskDelay(1000);
    }
}
