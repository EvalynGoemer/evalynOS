#include "badapple_audio.h"
#include "badapple_video.h"
#include "../../libc/string.h"
#include "../../kernel.h"
#include "../../hardware/pit.h"
#include "../../hardware/pcskpr.h"
#include "../../renderer/fb_renderer.h"
#include "../../interupts/interupts.h"

void playBadApple(char charBuffer[64]) {
    clearScreen(FB_WIDTH, FB_HEIGHT);

    printString("Kernel Shell> ", 10, FB_HEIGHT - 16);
    printString(charBuffer, 10, 8);
    printString("Started Playing BAD APPLE", 10, 8);
    printString(charBuffer, 10 + (14 * 8), FB_HEIGHT - 16);

    setup_pit(140);
    pitInteruptsTriggered = 0;

    int audioSamplesPlayed = 0;
    int lastTimeAudioSamplePlayed = 0;

    int frameIndex = 0;
    int desiredFrame = 0;
    int desiredFramePre = 0;

    while (audioSamplesPlayed < audio_data_length) {
        asm volatile("cli");
        while (pitInteruptsTriggered - lastTimeAudioSamplePlayed >= 1) {
            uint8_t bl = audio_data[audioSamplesPlayed];
            if (bl == 0xFF) {

            } else if (bl == 254) {
                stop_sound();
            } else {
                play_sound(audio_to_pcspkr[bl]);
            }

            lastTimeAudioSamplePlayed += 1;
            audioSamplesPlayed++;
        }

        if (audioSamplesPlayed >= 15499) {
            desiredFrame = (int)(((audioSamplesPlayed - 15499) / (13.5 / 3.0)) + desiredFramePre);
        } else {
            desiredFrame = (int)((audioSamplesPlayed / (14.0 / 3.0))) + 22;
            if (audioSamplesPlayed >= 15498) {
                desiredFramePre = desiredFrame;
            }
        }

        if (desiredFrame >= frame_count) {
            desiredFrame = frame_count;
        }

        if (desiredFrame != frameIndex) {
            frameIndex = desiredFrame;
            unsigned int start  = frame_offsets[frameIndex];
            unsigned int end    = (frameIndex + 1 < frame_count) ? frame_offsets[frameIndex + 1] : start;
            unsigned int length = end - start;

            drawFrame(&frames_rle[start], length, 120, 90, 8, 20, 3);
        }

        asm volatile("sti");
        __asm__ __volatile__("hlt");
    }

    stop_sound();

    pitInteruptsTriggered = 0;

    setup_pit(1000);
}
