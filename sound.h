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

void on_process(void *userdata);
#endif
