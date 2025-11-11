#include "fb_renderer.h"

#include "font8x8_basic.h"

#include "../filesystem/devfs/devfs.h"
#include "../libc/string.h"
#include "../libc/stdlib.h"

int TTY_WIDTH;
int TTY_HEIGHT;

char* termBuffer = NULL;
int termBufferSize = 0;
int termBufferIndex = 0;

int ttyDeviceRead(__attribute__((unused)) char* path, __attribute__((unused)) char* return_data,  __attribute__((unused)) int read_length) {
    return -1;
}

void render() {
    int x = 0;
    int y = 0;

    for (int i = 0; i < termBufferSize; i++) {
        printChar(termBuffer[i], x, y);

        x += FONT_WIDTH;
        if (x >= TTY_WIDTH * FONT_WIDTH) {
            x = 0;
            y += FONT_HEIGHT;
        }
    }
}

#define SCROLL_BUFFER() \
if (termBufferIndex >= termBufferSize && termBufferSize >= TTY_WIDTH) { \
    memmove(termBuffer, termBuffer + TTY_WIDTH, termBufferSize - TTY_WIDTH); \
    termBufferIndex = termBufferSize - TTY_WIDTH; \
    memset(termBuffer + termBufferIndex, '\0', TTY_WIDTH); \
}

int ttyDeviceWrite(__attribute__((unused)) char* path, char* write_data, int write_length) {
    for (int i = 0; write_length > i; i++) {
        switch (write_data[i]) {
            case '\x1b': // asni codes
                if (i + 3 < write_length) { // three byte codes
                    if (write_data[i + 1] == '[' && write_data[i + 2] == '2' && write_data[i + 3] == 'K') { // clear line
                        int startOfLine = termBufferIndex;
                        while (startOfLine % TTY_WIDTH) {
                            startOfLine--;
                        }

                        memset(termBuffer + startOfLine, '\0', TTY_WIDTH);
                        SCROLL_BUFFER();

                        i += 3;
                        continue;
                    }
                    if (write_data[i + 1] == '[' && write_data[i + 2] == '2' && write_data[i + 3] == 'J') { // clear screen
                        memset(termBuffer, '\0', termBufferSize);
                        SCROLL_BUFFER();
                        i += 3;
                        continue;
                    }
                }
                if (i + 2 < write_length) { // two byte codes
                    if (write_data[i + 1] == '[' && write_data[i + 2] == 'H') { // go home
                        termBufferIndex = 0;
                        i += 2;
                        continue;
                    }
                }
                break;
            case '\r':
                while (termBufferIndex % TTY_WIDTH) {
                    termBufferIndex--;
                }
                break;
            case '\n':
                while (termBufferIndex % TTY_WIDTH) {
                    termBufferIndex++;
                }

                SCROLL_BUFFER();

                break;
            case '\b':
                if (termBufferIndex > 0) {
                    termBuffer[--termBufferIndex] = '\0';
                }
                break;
            case '\0': {
                return i;
            }
            default:
                termBuffer[termBufferIndex] = write_data[i];

                termBufferIndex++;
                SCROLL_BUFFER();

                break;
        }
    }

    render();
    return write_length;
}

void setup_tty() {
    TTY_WIDTH = FB_WIDTH / FONT_WIDTH;
    TTY_HEIGHT = FB_HEIGHT / (FONT_HEIGHT);

    // HACK: Limit TTY width to 80 until ioctl to reserve regions of the framebuffer exists so that the bad apple demo can work
    if (TTY_WIDTH > 80) {
        TTY_WIDTH = 80;
    }

    termBuffer = malloc((TTY_WIDTH) * TTY_HEIGHT);
    termBufferSize = (TTY_WIDTH * TTY_HEIGHT);
    memset(termBuffer, '\0', termBufferSize);

    struct device* tty = malloc(sizeof(struct device));
    strcpy(tty->fullPath, "/dev/term/tty");
    tty->read = ttyDeviceRead;
    tty->write = ttyDeviceWrite;
    register_device(tty);
}

void clear_tty() {

}
