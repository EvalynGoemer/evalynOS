#include "badapple_audio.h"
#include "badapple_video.h"

#include <stdint.h>

static long syscall(int syscall_type, long a) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(syscall_type), "b"(a)
    );
    return ret;
}

int get_id() {
    return syscall(0, 0);
}

void print(char* string) {
    // do not call me
    // syscall(1, (long)string);
}

void play_sound(long dx) {
    syscall(10, dx);
}

void stop_sound() {
    syscall(11, 0);
}

void set_pit_frequency(int frequency) {
    syscall(20, frequency);
}

void reset_pit_cycles() {
    syscall(21, 0);
}

int get_pit_cycles() {
    return syscall(22, 0);
}


void setup_fb() {
    syscall(30, 0);
}

int get_pitch() {
    return syscall(31, 0);
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
    int thread_id = get_id() - 1;

    setup_fb();
    frameBufferPitch = get_pitch();

    while (1) {
        int pit_base = get_pit_cycles();

        int audioSamplesPlayed = 0;
        int lastTick = 0;

        int frameIndex = 0;
        int desiredFrame = 0;
        int desiredFramePre = 0;

        int accum = 0;

        while (audioSamplesPlayed < audio_data_length) {
            int currentTicks = get_pit_cycles() - pit_base;

            while (lastTick < currentTicks) {
                accum += 140;

                if (accum >= 1000) {
                    accum -= 1000;

                    uint8_t bl = audio_data[audioSamplesPlayed];
                    if (bl == 254) {
                        stop_sound();
                    } else if (bl != 0xFF) {
                        play_sound(audio_to_pcspkr[bl]);
                    }

                    audioSamplesPlayed++;
                    if (audioSamplesPlayed >= audio_data_length)
                        break;
                }

                lastTick++;
            }

            if (audioSamplesPlayed >= 15499) {
                desiredFrame = ((audioSamplesPlayed - 15499) * 2) / 9 + desiredFramePre;
            } else {
                desiredFrame = (audioSamplesPlayed * 3) / 14 + 22;
                if (audioSamplesPlayed >= 15498) {
                    desiredFramePre = desiredFrame;
                }
            }

            if (desiredFrame >= frame_count)
                desiredFrame = frame_count;

            if (desiredFrame != frameIndex) {
                frameIndex = desiredFrame;
                unsigned int start  = frame_offsets[frameIndex];
                unsigned int end    = (frameIndex + 1 < frame_count) ? frame_offsets[frameIndex + 1] : start;
                unsigned int length = end - start;

                drawFrame(&frames_rle[start], length, 120, 90, 650,
                          8 + (90 * 3 * thread_id), 3);
            }
        }

        stop_sound();
    }
}
