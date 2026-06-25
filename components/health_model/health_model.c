#include "health_model.h"

#include "esp_timer.h"

static int clamp_int(int value, int min_value, int max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

health_sample_t health_model_next_simulated_sample(void)
{
    static uint32_t tick;
    tick++;

    health_sample_t sample = {
        .heart_rate_bpm = 72 + (int)(tick % 9) - 4,
        .spo2_percent = 97 + (int)(tick % 3),
        .raw_red = 50000 + (tick * 137U) % 3000U,
        .raw_ir = 54000 + (tick * 173U) % 3500U,
        .timestamp_ms = esp_timer_get_time() / 1000,
        .valid = true,
        .simulated = true,
    };

    health_model_normalize(&sample);
    return sample;
}

void health_model_normalize(health_sample_t *sample)
{
    if (sample == 0) {
        return;
    }

    sample->heart_rate_bpm = clamp_int(sample->heart_rate_bpm, 40, 180);
    sample->spo2_percent = clamp_int(sample->spo2_percent, 70, 100);
}
