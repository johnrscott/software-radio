#include <stdint.h>
#include "driver/rmt_tx.h"
#include "esp_log.h"

#define RMT_LO_I_GPIO_NUM 8 // In-phase local oscillator output
#define RMT_LO_Q_GPIO_NUM 8 // Quadrature local oscillator output

#define TICK_FREQUENCY_HZ 80000000

static const char *TAG = "local_osc";

// RMT raw data buffer
static uint16_t lo_i_rmt_data[3]; // In-phase
static uint16_t lo_q_rmt_data[4]; // Quadrature

static rmt_channel_handle_t channel_i = NULL;
static rmt_channel_handle_t channel_q = NULL;

static rmt_encoder_handle_t copy_encoder = NULL;

/// Only call this function while the RMT is disabled
void write_frequency(uint16_t quarter_period_ticks) {
    // Set the in-phase data
    lo_i_rmt_data[0] = 0x8000 | (2*quarter_period_ticks); // 1 for 2N
    lo_i_rmt_data[1] = (2*quarter_period_ticks); // 0 for 2N
    lo_i_rmt_data[2] = 0;

    // Set the quadrature data
    lo_q_rmt_data[0] = quarter_period_ticks; // 0 for N
    lo_q_rmt_data[1] = 0x8000 | (2*quarter_period_ticks); // 1 for 2N
    lo_q_rmt_data[2] = quarter_period_ticks; // 0 for N
    lo_i_rmt_data[3] = 0;
}

static void make_rmt_channel(gpio_num_t gpio_num, rmt_channel_handle_t *channel) {
    ESP_LOGI(TAG, "Create RMT TX channel");
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = gpio_num,
        .mem_block_symbols = 64,
        .resolution_hz = TICK_FREQUENCY_HZ,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, channel));
}

static void enable_channels() {
    ESP_ERROR_CHECK(rmt_enable(channel_i));
    ESP_ERROR_CHECK(rmt_enable(channel_q));    
}

static void setup_copy_encoder() {
    copy_encoder = NULL;
    rmt_copy_encoder_config_t encoder_config; // Currently unused argument
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&encoder_config, &copy_encoder));
}

static void disable_local_osc() {
    ESP_ERROR_CHECK(rmt_enable(channel_i));
    ESP_ERROR_CHECK(rmt_enable(channel_q));
}

static void enable_local_osc() {

    enable_channels();

    rmt_transmit_config_t tx_config = {
        .loop_count = -1, // infinite loop
    };
    ESP_ERROR_CHECK(rmt_transmit(channel_i, copy_encoder,
				 &lo_i_rmt_data, sizeof(lo_i_rmt_data),
				 &tx_config));
    ESP_ERROR_CHECK(rmt_transmit(channel_q, copy_encoder,
				 &lo_q_rmt_data, sizeof(lo_q_rmt_data),
				 &tx_config));    
}

void setup_local_osc() {

    ESP_LOGI(TAG, "Create the in-phase and quadrature channels");
    make_rmt_channel(RMT_LO_I_GPIO_NUM, &channel_i);
    make_rmt_channel(RMT_LO_Q_GPIO_NUM, &channel_q);

    ESP_LOGI(TAG, "Install copy encoder");
    setup_copy_encoder();

    ESP_LOGI(TAG, "Enable RMT TX channel");    
}
