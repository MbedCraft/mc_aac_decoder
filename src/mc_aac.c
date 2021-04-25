/* ------------------------------------------------------------------------- *\
 * Standard Includes
 * ------------------------------------------------------------------------- */
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

/* ------------------------------------------------------------------------- *\
 * MbedCraft includes
 * ------------------------------------------------------------------------- */
#include "mc_aac.h"
#include "mc_assert.h"
#include "libhelix_HAACDECODER/aac_decoder.h"

/* ------------------------------------------------------------------------- *\
 * Static variable definitions
 * ------------------------------------------------------------------------- */
static mc_aac_play_callback_t __play_callback = NULL;
static mc_aac_config_callback_t __config_callback = NULL;

/* ------------------------------------------------------------------------- *\
 * Public function implementations
 * ------------------------------------------------------------------------- */
void mc_aac_init(
        mc_aac_config_callback_t config_callback,
        mc_aac_play_callback_t play_callback) {
    __config_callback = config_callback;
    __play_callback = play_callback;
}

int mc_aac_play_file(const char * const filename) {
    const int max_in_frame_size = 2000; // FIXME calibrate the frame size
    const int max_out_frame_size = 2 * 1024 * 32/8;
    int channels;
    int sample_rate;
    int bits_per_sample;
    int out_frame_size;
    int sound_file;
    int ret = -1;
    int aac_ret;
    int bytes_left;
    ssize_t read_bytes;
    ssize_t offset;
    uint8_t *inbuf = NULL;
    short *outbuf = NULL;
    bool eof;

    sound_file = open(filename, O_RDONLY);

    ASSERTW_RET(sound_file > 0, -1, "Could not open the file %s", filename);

    do {
        offset = 0;

        inbuf = malloc(max_in_frame_size);
        ASSERTW_BRK(NULL != inbuf, "Could not allocate the input buffer");
        outbuf = malloc(max_out_frame_size);
        ASSERTW_BRK(NULL != inbuf, "Could not allocate the input buffer");
        ASSERTW_BRK(true == AACDecoder_AllocateBuffers(),
                "Could not allocate AAC buffers");

        do {
            read_bytes = max_in_frame_size - offset;
            read_bytes = read(sound_file, inbuf+offset, read_bytes);
            ASSERTW_BRK(0 <= read_bytes, "Error while reading the sound file");
            if (read_bytes == 0) { eof = true; } // EOF
            else { eof = false; }

            read_bytes += offset;
            bytes_left = read_bytes;
            aac_ret = AACDecode(inbuf, &bytes_left, outbuf);

            ASSERTW_BRK(aac_ret >= 0,
                    "AAC decoder returned: %d", aac_ret);
            // If bytes_left has not been changed, there is a problem ...
            ASSERTW_BRK(bytes_left != read_bytes,
                    "AAC decoder internal error");

            channels = AACGetChannels();
            sample_rate = AACGetSampRate();
            bits_per_sample = AACGetBitsPerSample();

            // Each decoded frame contains 1024 samples
            out_frame_size = (bits_per_sample / 8) * AACGetOutputSamps();

            if (__config_callback) {
                __config_callback(sample_rate, bits_per_sample, channels);
            }
            if (__play_callback) {
                __play_callback((const uint8_t *) outbuf, out_frame_size);
            }

            // Move the remaining data at the beginning of the buffer
            offset = read_bytes - bytes_left;
            memmove(inbuf, inbuf + offset, bytes_left);
            offset = bytes_left;
        } while ((eof == false) || (bytes_left != 0));

        ret = 0;
    } while (0);

    if (NULL != inbuf) free(inbuf);
    if (NULL != outbuf) free(outbuf);
    AACDecoder_FreeBuffers();
    close(sound_file);

    return ret;
}
