#include "sound.h"



/* [on_process] */



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