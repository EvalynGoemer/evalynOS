#include "renderer/fb_renderer.h"
#include "renderer/kpanic_image.h"
#include "hardware/pcskpr.h"

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
void panic(char* message) {
    asm volatile("cli");
    drawImage(kernel_panic_image, kernel_panic_image_data_width, kernel_panic_image_data_height, 0, 0, 60);
    printString("The kernel is fucked dawg", 500 , 300 );
    printString(message, 500 , 310 );

    play_sound(1000);

    while (1) {

    }
}
