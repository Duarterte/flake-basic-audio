#include "sound.h"

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