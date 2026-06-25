#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int heart_rate_bpm;
    int spo2_percent;
    uint32_t raw_red;
    uint32_t raw_ir;
    int64_t timestamp_ms;
    bool valid;
    bool simulated;
} health_sample_t;

health_sample_t health_model_next_simulated_sample(void);
void health_model_normalize(health_sample_t *sample);
