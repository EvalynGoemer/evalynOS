#include "../kernel.h"
#include "../libc/string.h"
#include "badapple/badapple.h"

int handle_demos(char charBuffer[64]) {
    if(strncmp("BADAPPLE", charBuffer, 8) == 0) {
        playBadApple(charBuffer);
        return 1;
    }
    return 0;
}
