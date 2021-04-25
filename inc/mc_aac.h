#pragma once

typedef void (* mc_aac_play_callback_t) (const uint8_t *buf, uint32_t len);
typedef void (* mc_aac_config_callback_t) (
        int sample_rate,
        int bits_per_sample,
        int channels);

void mc_aac_init(
        mc_aac_config_callback_t config_callback,
        mc_aac_play_callback_t play_callback);
int mc_aac_play_file(const char * const filename);

