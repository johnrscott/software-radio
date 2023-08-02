#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"

#define RMT_LO_I_GPIO_NUM 8 // In-phase local oscillator output
#define RMT_LO_Q_GPIO_NUM 8 // Quadrature local oscillator output

#define TICK_FREQUENCY_HZ 40000000 


static const char *TAG = "sdr";

void app_main(void)
{
    ESP_LOGI(TAG, "Started program");

    ESP_LOGI(TAG, "Create RMT TX channel");
    rmt_channel_handle_t channel_handle = NULL;
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = RMT_LO_I_GPIO_NUM,
        .mem_block_symbols = 64,
        .resolution_hz = TICK_FREQUENCY_HZ,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &channel_handle));

    ESP_LOGI(TAG, "Install copy encoder");
    rmt_encoder_handle_t copy_encoder = NULL;
    rmt_copy_encoder_config_t encoder_config; // Currently unused argument
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&encoder_config, &copy_encoder));

    
    ESP_LOGI(TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(channel_handle));

    rmt_transmit_config_t tx_config = {
        .loop_count = -1, // infinite loop
    };

    uint32_t raw_rmt_data[3] = {
	0xffff7fff,
	0 // End of transmission marker
    };

    ESP_ERROR_CHECK(rmt_transmit(channel_handle, copy_encoder,
				 &raw_rmt_data, sizeof(raw_rmt_data),
				 &tx_config)); 
    
    while (1) {
	vTaskDelay(1000);
    }
}
