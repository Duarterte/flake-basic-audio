#include "sound.h"

double frequency = 440.00;
atomic_bool running = ATOMIC_VAR_INIT(true);
struct pw_stream *stream = NULL;
struct pw_main_loop *main_loop = NULL;

void on_process(void *userdata)
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