#include "badapple_audio.h"
#include "badapple_video.h"

#include <stdint.h>
#include <stddef.h>

static long syscall(int syscall_type, long a, long b) {
    long ret;
    __asm__ volatile (
        "int $0x69"
        : "=a"(ret)
        : "a"(syscall_type), "b"(a), "c"(b)
        : "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
    return ret;
}

void play_sound(long dx) {
    syscall(10, dx, 0);
}

void stop_sound() {
    syscall(11, 0, 0);
}

void set_pit_frequency(int frequency) {
    syscall(20, frequency, 0);
}

void reset_pit_cycles() {
    syscall(21, 0, 0);
}

int get_pit_cycles() {
    return syscall(22, 0, 0);
}


void setup_fb() {
    syscall(30, 0, 0);
}

int get_pitch() {
    return syscall(31, 0, 0);
}

uint64_t frameBufferBase = 0x00000000A0000000;
uint32_t frameBufferPitch;

void plotPixel(int x, int y, int color) {
    *((volatile uint32_t*)frameBufferBase + y * (frameBufferPitch >> 2) + x) = color;
}

static unsigned int prevFrame[120 * 90];
void drawFrame(const unsigned char* rleData, int rleLength, int frameWidth, int frameHeight, int startX, int startY, int scale) {
    int x = 0, y = 0;
    int pos = 0;

    while (pos + 1 < rleLength && y < frameHeight) {
        int count = rleData[pos++];
        unsigned char value = rleData[pos++];
        unsigned int color = value ? 0xFFFFFFFF : 0x00000000;

        for (int i = 0; i < count && y < frameHeight; i++) {
            int idx = y * frameWidth + x;

            if (prevFrame[idx] != color) {
                prevFrame[idx] = color;

                int px = startX + x * scale;
                int py = startY + y * scale;
                int scaledWidth = scale;
                int scaledHeight = scale;

                for (int dy = 0; dy < scaledHeight; dy++) {
                    int rowY = py + dy;
                    for (int i = 0; i < scaledWidth; i++) {
                        plotPixel(px + i, rowY, color);
                    }
                }
            }

            x++;
            if (x >= frameWidth) {
                x = 0;
                y++;
            }
        }
    }
}

int main() __attribute__((section(".entry")));
int main() {
    setup_fb();
    frameBufferPitch = get_pitch();

    while(1) {
        set_pit_frequency(140);
        reset_pit_cycles();

        int audioSamplesPlayed = 0;
        int lastTimeAudioSamplePlayed = 0;

        int frameIndex = 0;
        int desiredFrame = 0;
        int desiredFramePre = 0;

        while (audioSamplesPlayed < audio_data_length) {
            while (get_pit_cycles() - lastTimeAudioSamplePlayed >= 1) {
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

                drawFrame(&frames_rle[start], length, 120, 90, 650, 8, 3);
            }
        }

        stop_sound();

        reset_pit_cycles();

        set_pit_frequency(1000);
    }
}
