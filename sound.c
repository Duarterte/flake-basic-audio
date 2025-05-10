#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>
#include <stdatomic.h>


#define DEFAULT_RATE 44100
#define DEFAULT_CHANNELS 2
#define DEFAULT_VOLUME 0.7

enum
{

        TONE_C,
        TONE_D,
        TONE_E,
        TONE_F,
        TONE_G,
        TONE_A,
        TONE_B
};

static const double tones[] = {261.6256, 293.6648, 329.6276, 349.2282, 391.9954, 440.00, 493.8833};

static atomic_bool running = ATOMIC_VAR_INIT(true);
static struct pw_stream *stream = NULL;
static struct pw_main_loop *main_loop = NULL;

double frequency = 440.00;

struct data
{
        struct pw_main_loop *loop;
        struct pw_stream *stream;
        double accumulator;
};

static void on_process(void *userdata)
{
        struct data *data = userdata;
        struct pw_buffer *b;
        struct spa_buffer *buf;
        int i, c, n_frames, stride;
        int16_t *dst, val;

        if ((b = pw_stream_dequeue_buffer(data->stream)) == NULL)
        {
                pw_log_warn("out of buffers: %m");
                return;
        }

        buf = b->buffer;
        if ((dst = buf->datas[0].data) == NULL)
                return;

        stride = sizeof(int16_t) * DEFAULT_CHANNELS;
        n_frames = buf->datas[0].maxsize / stride;
        if (b->requested)
                n_frames = SPA_MIN(b->requested, n_frames);

        for (i = 0; i < n_frames; i++)
        {
                data->accumulator += M_PI_M2 * frequency / DEFAULT_RATE;
                if (data->accumulator >= M_PI_M2)
                        data->accumulator -= M_PI_M2;
                val = sin(data->accumulator) * DEFAULT_VOLUME * 32767.0;
                for (c = 0; c < DEFAULT_CHANNELS; c++)
                        *dst++ = val;
        }

        buf->datas[0].chunk->offset = 0;
        buf->datas[0].chunk->stride = stride;
        buf->datas[0].chunk->size = n_frames * stride;

        pw_stream_queue_buffer(data->stream, b);
}
/* [on_process] */

static const struct pw_stream_events stream_events = {
    PW_VERSION_STREAM_EVENTS,
    .process = on_process,
};

void *wait_n_stop(void *arg)
{
        uint32_t t = *(uint32_t *)arg;
        usleep(1000 * t);
        atomic_store(&running, false);
        if (stream) {
                pw_stream_disconnect(stream);
                pw_stream_destroy(stream);
                stream = NULL;
    }
}

void playSound(double freq, uint32_t duration)
{
        struct data data = {
            0,
        };
        const struct spa_pod *params[1];

        
        // Reset the data structure for new note
        data.accumulator = 0; // Reset the phase accumulator
        frequency = freq;      // Set the new frequency
        // Create new stream if it doesn't exist or was destroyed
        if (data.stream == NULL)
        {
                const struct spa_pod *params[1];
                uint8_t buffer[1024];
                struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

                data.stream = pw_stream_new_simple(
                    pw_main_loop_get_loop(main_loop),
                    "audio-src",
                    pw_properties_new(
                        PW_KEY_MEDIA_TYPE, "Audio",
                        PW_KEY_MEDIA_CATEGORY, "Playback",
                        PW_KEY_MEDIA_ROLE, "Music",
                        NULL),
                    &stream_events,
                    &data);

                params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat,
                                                       &SPA_AUDIO_INFO_RAW_INIT(
                                                               .format = SPA_AUDIO_FORMAT_S16,
                                                               .channels = DEFAULT_CHANNELS,
                                                               .rate = DEFAULT_RATE));

                pw_stream_connect(data.stream,
                                  PW_DIRECTION_OUTPUT,
                                  PW_ID_ANY,
                                  PW_STREAM_FLAG_AUTOCONNECT |
                                      PW_STREAM_FLAG_MAP_BUFFERS |
                                      PW_STREAM_FLAG_RT_PROCESS,
                                  params, 1);
        }

        // Start playing
        atomic_store(&running, true);

        // Create stop thread
        uint32_t duration_copy = duration;
        pthread_t stop_thread;
        pthread_create(&stop_thread, NULL, wait_n_stop, &duration_copy);

        // Main loop
        struct pw_loop *loop = pw_main_loop_get_loop(main_loop);
        while (atomic_load(&running))
        {
                pw_loop_iterate(loop, 0);
        }

        // Wait for stop thread
        pthread_join(stop_thread, NULL);

        if (data.stream)
        {
                pw_stream_disconnect(data.stream);
        }

}

int main(int argc, char *argv[])
{
        pw_init(NULL, NULL);

        main_loop = pw_main_loop_new(NULL);

        playSound(tones[TONE_C], 1000);
        sleep(1);
        playSound(tones[TONE_D], 1000);
        sleep(1);
        playSound(tones[TONE_E], 1000);
        sleep(1);
        playSound(tones[TONE_F], 1000);
        sleep(1);
        playSound(tones[TONE_G], 1000);
        sleep(1);
        playSound(tones[TONE_A], 1000);
        sleep(1);
        playSound(tones[TONE_B], 1000);
        sleep(1);
        // pthread_create(&thread1, NULL, stop_after_second, NULL);

        pw_deinit();

        return 0;
}