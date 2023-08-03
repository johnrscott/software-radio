
// RMT raw data buffer
static uint16_t lo_i_rmt_data[3]; // In-phase
static uint16_t lo_q_rmt_data[4]; // Quadrature

void write_frequency(uint16_t quarter_period_ticks) {
    // Set the in-phase data
    lo_i_rmt_data[0] = 0x8000 | quarter_period_ticks;
    lo_i_rmt_data[1] = quarter_period_ticks;
    lo_i_rmt_data[1] = 0;
}

