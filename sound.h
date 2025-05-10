#ifndef SOUND_H
#define SOUND_H

#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>
#include <stdatomic.h>

#define M_PI_M2 (M_PI + M_PI)

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

extern atomic_bool running;
extern struct pw_stream *stream;
extern struct pw_main_loop *main_loop;
extern double frequency;

struct data
{
        struct pw_main_loop *loop;
        struct pw_stream *stream;
        double accumulator;
};

void on_process(void *userdata);
void *wait_n_stop(void *arg);
void playSound(double freq, uint32_t duration);

static const struct pw_stream_events stream_events = {
    PW_VERSION_STREAM_EVENTS,
    .process = on_process,
};


#endif
