#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "string.h"

#define TERM_WIDTH 80
#define TERM_HEIGHT 24
char terminal[TERM_HEIGHT][TERM_WIDTH];
int termY = 0;
int term_updated = 0;
int printf(const char* fmt, ...) {
    char tmp[TERM_WIDTH];
    int ix = 0;

    va_list args;
    va_start(args, fmt);

    for (int i = 0; fmt[i] != '\0' && ix < TERM_WIDTH - 1; i++) {
        if (fmt[i] == '%') {
            i++;

            if (fmt[i] == '%') {
                tmp[ix++] = '%';
                continue;
            }

            if (fmt[i] == 's') {
                const char* str = va_arg(args, const char*);
                if (!str) str = "(null)";
                for (int j = 0; str[j] && ix < TERM_WIDTH - 1; j++) tmp[ix++] = str[j];
                continue;
            }

            int neg = 0;
            unsigned long long unum = 0;

            if (fmt[i] == 'l') {
                if (fmt[i + 1] == 'l') {
                    if (fmt[i + 2] == 'd' || fmt[i + 2] == 'i') {
                        long long num = va_arg(args, long long);
                        neg = num < 0;
                        unum = neg ? -num : num;
                        i += 2;
                    } else if (fmt[i + 2] == 'u') {
                        unum = va_arg(args, unsigned long long);
                        i += 2;
                    } else if (fmt[i + 2] == 'x') {
                        unum = va_arg(args, unsigned long long);
                        char hex[] = "0123456789ABCDEF";
                        char out[17];
                        int oix = 16;
                        out[oix] = '\0';
                        if (unum == 0) out[--oix] = '0';
                        while (unum) {
                            out[--oix] = hex[unum & 0xF];
                            unum >>= 4;
                        }
                        for (int j = oix; out[j] && ix < TERM_WIDTH - 1; j++) tmp[ix++] = out[j];
                        i += 2;
                        continue;
                    }
                } else {
                    if (fmt[i + 1] == 'd' || fmt[i + 1] == 'i') {
                        long num = va_arg(args, long);
                        neg = num < 0;
                        unum = neg ? -num : num;
                        i += 1;
                    } else if (fmt[i + 1] == 'u') {
                        unum = va_arg(args, unsigned long);
                        i += 1;
                    } else if (fmt[i + 1] == 'x') {
                        unum = va_arg(args, unsigned long);
                        char hex[] = "0123456789ABCDEF";
                        char out[17];
                        int oix = 16;
                        out[oix] = '\0';
                        if (unum == 0) out[--oix] = '0';
                        while (unum) {
                            out[--oix] = hex[unum & 0xF];
                            unum >>= 4;
                        }
                        for (int j = oix; out[j] && ix < TERM_WIDTH - 1; j++) tmp[ix++] = out[j];
                        i += 1;
                        continue;
                    }
                }
            } else {
                if (fmt[i] == 'd' || fmt[i] == 'i') {
                    int num = va_arg(args, int);
                    neg = num < 0;
                    unum = neg ? -num : num;
                } else if (fmt[i] == 'u') {
                    unum = va_arg(args, unsigned int);
                } else if (fmt[i] == 'x') {
                    unsigned int num = va_arg(args, unsigned int);
                    char hex[] = "0123456789ABCDEF";
                    char out[9];
                    int oix = 8;
                    out[oix] = '\0';
                    if (num == 0) out[--oix] = '0';
                    while (num) {
                        out[--oix] = hex[num & 0xF];
                        num >>= 4;
                    }
                    for (int j = oix; out[j] && ix < TERM_WIDTH - 1; j++) tmp[ix++] = out[j];
                    continue;
                }
            }

            char out[21];
            int oix = 20;
            out[oix] = '\0';
            if (unum == 0) out[--oix] = '0';
            while (unum && oix > 0) {
                out[--oix] = '0' + (unum % 10);
                unum /= 10;
            }
            if (neg) out[--oix] = '-';
            for (int j = oix; out[j] && ix < TERM_WIDTH - 1; j++) tmp[ix++] = out[j];

        } else {
            tmp[ix++] = fmt[i];
        }
    }

    tmp[ix] = '\0';
    va_end(args);

    if (termY >= TERM_HEIGHT) {
        memmove(terminal[0], terminal[1], sizeof(char) * TERM_WIDTH * (TERM_HEIGHT - 1));
        termY = TERM_HEIGHT - 1;
    }

    strcpy(terminal[termY], "                                                                                ");
    strcpy(terminal[termY], tmp);
    termY++;
    term_updated = 1;

    return 0;
}

void cleark() {
    memset(terminal, 0, sizeof(terminal));
    termY = 0;
}
