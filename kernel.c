#include <stdint.h>
#include "font8x8_basic.h"

uint64_t frameBufferBase;
uint32_t pixelsPerScanLine;

void plotPixel(int x, int y, int color) {
    *((uint32_t*)(frameBufferBase + 4 * pixelsPerScanLine * y + 4 * x)) = color;
}

void printChar(char c, int x, int y) {
    int index = c;
    for (int row = 0; row < 8; row++) {
        unsigned char bits = font8x8_basic[index][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << col)) {
                plotPixel(x + col, y + row, 0xFFFFFFFF);
            } else {
                plotPixel(x + col, y + row, 0x00000000);
            }
        }
    }
}

void printString(const char* str, int x, int y) {
    while (*str) {
        printChar(*str++, x, y);
        x += 8;
    }
}

void drawImage(const uint32_t* image, int width, int height, int x0, int y0) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int color = image[y * width + x];
            plotPixel(x0 + x, y0 + y, color);
        }
    }
}

void clearScreen(int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            plotPixel(x, y, 0x00000000);
        }
    }
    printString("Kernel Debug Info", 10, 1080 - 64);
    printString("genericInteruptsTriggered: [Not Updated]", 10, 1080 - 56);
    printString("pitInteruptsTriggered:", 10, 1080 - 48);
    printString("ps2InteruptsTriggered: [Not Updated]", 10, 1080 - 40);
    printString("PS/2 Keyboard Event: [Not Updated]", 10, 1080 - 32);
}

int nnstrcmp(const char *input, const char *reference) {
    int i = 0;
    while (input[i] != '\0') {
        if (input[i] != reference[i]) {
            return (unsigned char)input[i] - (unsigned char)reference[i];
        }
        i++;
    }
    return 0;
}

struct __attribute__((packed)) GDTEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
};

struct __attribute__((packed)) GDTR {
    uint16_t limit;
    uint64_t base;
};

struct __attribute__((packed)) IDTEntry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
};

struct __attribute__((packed)) IDTR {
    uint16_t limit;
    uint64_t base;
};

static struct GDTEntry gdt[3];
static struct GDTR gdtr;

static struct IDTEntry idt[256];
static struct IDTR idtr;

static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ( "inb %1, %0"
    : "=a"(ret)
    : "Nd"(port) );
    return ret;
}

int genericInteruptsTriggered = 0;
__attribute__((interrupt))
void default_isr(void* frame) {
    genericInteruptsTriggered++;
    char result[2048];
    int num = genericInteruptsTriggered;

    char *p = result;
    const char *base = "genericInteruptsTriggered: ";
    while (*base) *p++ = *base++;

    char temp[256];
    int i = 0;
    if (num == 0) {
        temp[i++] = '0';
    } else {
        while (num > 0) {
            temp[i++] = (num % 10) + '0';
            num /= 10;
        }
    }

    while (i > 0) {
        *p++ = temp[--i];
    }
    *p = '\0';

    printString("                                                                ", 10, 1080 - 56);
    printString(result, 10, 1080 - 56);

    outb(0x20,0x20);
    outb(0xa0,0x20);
}

int pitInteruptsTriggered = 0;
__attribute__((interrupt))
void pit_isr(void* frame) {
    pitInteruptsTriggered++;

    char result[2048];
    int num = pitInteruptsTriggered;

    char *p = result;
    const char *base = "pitInteruptsTriggered: ";
    while (*base) *p++ = *base++;

    char temp[256];
    int i = 0;
    if (num == 0) {
        temp[i++] = '0';
    } else {
        while (num > 0) {
            temp[i++] = (num % 10) + '0';
            num /= 10;
        }
    }

    while (i > 0) {
        *p++ = temp[--i];
    }
    *p = '\0';

    printString(result, 10, 1080 - 48);

    outb(0x20,0x20);
    outb(0xa0,0x20);
}

int ps2InteruptsTriggered = 0;
unsigned char ps2LastScanCode = 0;
int ps2IrqFired = 0;
__attribute__((interrupt))
void ps2_isr(void* frame) {
    ps2InteruptsTriggered++;
    ps2IrqFired = 1;

    char result[2048];
    int num = ps2InteruptsTriggered;

    char *p = result;
    const char *base = "ps2InteruptsTriggered: ";
    while (*base) *p++ = *base++;

    char temp[256];
    int i = 0;
    if (num == 0) {
        temp[i++] = '0';
    } else {
        while (num > 0) {
            temp[i++] = (num % 10) + '0';
            num /= 10;
        }
    }

    while (i > 0) {
        *p++ = temp[--i];
    }
    *p = '\0';

    printString("                                                                ", 10, 1080 - 40);
    printString(result, 10, 1080 - 40);

    char result2[512];
    unsigned char num2 = inb(0x60);
    ps2LastScanCode = num2;
    char *p2 = result2;
    const char *base2 = "PS/2 Keyboard Event: ";
    while (*base2) *p2++ = *base2++;

    char temp2[64];
    int i2 = 0;
    if (num2 == 0) {
        temp2[i2++] = '0';
    } else {
        while (num2 > 0) {
            temp2[i2++] = (num2 % 10) + '0';
            num2 /= 10;
        }
    }

    while (i2 > 0) {
        *p2++ = temp2[--i2];
    }
    *p2 = '\0';

    printString("                                                                ", 10, 1080 - 32);
    printString(result2, 10, 1080 - 32);


    outb(0x20,0x20);
    outb(0xa0,0x20);
}

void setup_gdt() {
    gdt[0] = (struct GDTEntry){ 0 }; // Null

    // Code Segment: base=0, limit=0xFFFFF, access=0x9A, gran=0x20
    gdt[1] = (struct GDTEntry){
        .limit_low = 0,
        .base_low = 0,
        .base_mid = 0,
        .access = 0x9A,             // Present, ring 0, code, executable, readable
        .granularity = 0x20,        // Long mode
        .base_high = 0
    };

    // Data Segment: access=0x92, gran=0x00
    gdt[2] = (struct GDTEntry){
        .limit_low = 0,
        .base_low = 0,
        .base_mid = 0,
        .access = 0x92,             // Present, ring 0, data, writable
        .granularity = 0x00,
        .base_high = 0
    };

    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uint64_t)&gdt;

    asm volatile ("lgdt %0" : : "m"(gdtr));
    asm volatile (
        "mov $0x10, %%ax \n"
        "mov %%ax, %%ds \n"
        "mov %%ax, %%ss \n"
        "mov %%ax, %%es \n"
        "mov %%ax, %%fs \n"
        "mov %%ax, %%gs \n"
        "pushq $0x08 \n"
        "lea 1f(%%rip), %%rax \n"
        "pushq %%rax \n"
        "lretq \n"
        "1:\n"
        : : : "rax"
    );
}

void set_idt_entry(int index, void (*handler)()) {
    uint64_t addr = (uint64_t)handler;

    idt[index] = (struct IDTEntry){
        .offset_low = addr & 0xFFFF,
        .selector = 0x08,
        .ist = 0,
        .type_attr = 0x8E, // Interrupt gate, ring 0, present
        .offset_mid = (addr >> 16) & 0xFFFF,
        .offset_high = (addr >> 32),
        .zero = 0
    };
}

void setup_idt() {
    for (int i = 0; i < 256; ++i)
        set_idt_entry(i, (void (*)())default_isr);

    set_idt_entry(0x20, (void (*)())pit_isr);
    set_idt_entry(0x21, (void (*)())ps2_isr);

    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)&idt;

    asm volatile ("lidt %0" : : "m"(idtr));
}

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01

#define ICW4_8086    0x01

void setup_pic(int offset1, int offset2) {
    // Save masks
    unsigned char a1 = inb(PIC1_DATA);
    unsigned char a2 = inb(PIC2_DATA);

    // Start initialization sequence
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    // Set vector offsets
    outb(PIC1_DATA, offset1); // Master PIC vector offset
    outb(PIC2_DATA, offset2); // Slave PIC vector offset

    // Tell Master PIC about Slave PIC at IRQ2 (0000 0100)
    outb(PIC1_DATA, 0x04);
    // Tell Slave PIC its cascade identity (0000 0010)
    outb(PIC2_DATA, 0x02);

    // Set PICs to 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // Restore saved masks
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

#define PIT_CONTROL_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIT_FREQUENCY 1193182
#define IRQ0_VECTOR 32

void setup_pit(unsigned int frequency) {
    unsigned int divisor = PIT_FREQUENCY / frequency;
    outb(PIT_CONTROL_PORT, 0x36);          // Channel 0, access mode: lobyte/hibyte, mode 2
    outb(PIT_CHANNEL0_PORT, divisor & 0xFF);  // Low byte of divisor
    outb(PIT_CHANNEL0_PORT, divisor >> 8);    // High byte of divisor

    // Unmask IRQ0 (timer)
    unsigned char mask = inb(0x21);
    mask = inb(PIC1_DATA);
    mask &= ~(1 << 0);
    outb(0x21, mask);
}

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64
#define PS2_COMMAND_PORT 0x64

#define PS2_STATUS_INPUT_BUFFER_FULL 0x02
#define PS2_STATUS_OUTPUT_BUFFER_FULL 0x01

static void ps2_wait_input_empty() {
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_BUFFER_FULL);
}

static void ps2_wait_output_full() {
    while (!(inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_BUFFER_FULL));
}

void setup_ps2() {
    ps2_wait_input_empty();
    outb(PS2_COMMAND_PORT, 0xAD); // Disable first PS/2 port
    ps2_wait_input_empty();
    outb(PS2_COMMAND_PORT, 0xA7); // Disable second PS/2 port

    while (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_BUFFER_FULL)
        (void)inb(PS2_DATA_PORT);

    ps2_wait_input_empty();
    outb(PS2_COMMAND_PORT, 0xAA);
    ps2_wait_output_full();
    unsigned char result = inb(PS2_DATA_PORT);
    if (result != 0x55) {
        return;
    }
    ps2_wait_input_empty();
    outb(PS2_COMMAND_PORT, 0xAE); // Enable first PS/2 port


    ps2_wait_input_empty();
    outb(PS2_COMMAND_PORT, 0x20);  // Read Controller Configuration Byte
    ps2_wait_output_full();
    unsigned char config = inb(PS2_DATA_PORT);

    config |= 1 << 0;  // Enable IRQ1 (keyboard)

    ps2_wait_input_empty();
    outb(PS2_COMMAND_PORT, 0x60);  // Write Controller Configuration Byte
    ps2_wait_input_empty();
    outb(PS2_DATA_PORT, config);

    ps2_wait_input_empty();
    outb(PS2_DATA_PORT, 0xF4); // Send "Enable Scanning" command to keyboard

    // Unmask IRQ1 (keyboard)
    unsigned char mask;
    mask = inb(PIC1_DATA);
    mask &= ~(1 << 1);
    outb(PIC1_DATA, mask);
}

void kernel_main(uint64_t fbb, uint32_t pps) __attribute__((section(".entry")));
void kernel_main(uint64_t fbb, uint32_t pps) {
    frameBufferBase = fbb;
    pixelsPerScanLine = pps;

    genericInteruptsTriggered = 0;
    pitInteruptsTriggered = 0;
    ps2InteruptsTriggered = 0;
    ps2LastScanCode = 0;
    ps2IrqFired = 0;

    printString("Kernel: Kernel Started", 10, 10);
    printString("Kernel Debug Info", 10, 1080 - 64);

    setup_gdt();
    setup_idt();
    printString("Kernel: Basic GDT & IDT Setup", 10, 10 + 8);
    printString("genericInteruptsTriggered: [Not Updated]", 10, 1080 - 56);


    setup_pic(0x20, 0x28);
    printString("Kernel: PIC Setup", 10, 10 + 16);

    setup_pit(1000);
    printString("Kernel: PIT Setup", 10, 10 + 24);
    printString("pitInteruptsTriggered:", 10, 1080 - 48);

    setup_ps2();
    printString("Kernel: PS/2 Keyboard Setup", 10, 10 + 32);
    printString("ps2InteruptsTriggered: [Not Updated]", 10, 1080 - 40);
    printString("PS/2 Keyboard Event: [Not Updated]", 10, 1080 - 32);

    printString("Kernel: No Block Storage Device or Ramdisk With Supported Filesystem & Init Program Found; Press Enter to Start the Builtin Kernel Shell", 10, 10 + 40);

    while (ps2LastScanCode != 28) {
        __asm__ __volatile__("hlt");
    }

    ps2LastScanCode = 0;

    clearScreen(1920, 1080);

    static const unsigned char scancode_set1_ascii[256] = { [0x02]='1',[0x03]='2',[0x04]='3',[0x05]='4',[0x06]='5',[0x07]='6',[0x08]='7',[0x09]='8',[0x0A]='9',[0x0B]='0',[0x0C]='-',[0x0D]='=',[0x10]='Q',[0x11]='W',[0x12]='E',[0x13]='R',[0x14]='T',[0x15]='Y',[0x16]='U',[0x17]='I',[0x18]='O',[0x19]='P',[0x1A]='[',[0x1B]=']',[0x1E]='A',[0x1F]='S',[0x20]='D',[0x21]='F',[0x22]='G',[0x23]='H',[0x24]='J',[0x25]='K',[0x26]='L',[0x27]=';',[0x28]='\'',[0x29]='`',[0x2C]='Z',[0x2D]='X',[0x2E]='C',[0x2F]='V',[0x30]='B',[0x31]='N',[0x32]='M',[0x33]=',',[0x34]='.',[0x35]='/',[0x39]=' ' };

    char charBuffer[256];

    for (int i = 0; i < 220; ++i) {
        charBuffer[i] = ' ';
    }

    int charBufferIndex = 0;

    while (1) {
        if (ps2IrqFired) {
            printString("Kernel Shell> ", 10, 1080 - 16);
            ps2IrqFired = 0;

            if (ps2LastScanCode == 14) { // Backspace
                if (charBufferIndex > 0) {
                    charBufferIndex--;
                    charBuffer[charBufferIndex] = ' ';
                    printString(charBuffer, 10 + (14 * 8), 1080 - 16);
                }
            } else if(ps2LastScanCode == 28) { // Enter
                if(nnstrcmp("TEST ", charBuffer) == 0) {
                    for (int i = 0; i < 220 - 1; ++i) {
                        charBuffer[i] = ' ';
                    }
                    printString(charBuffer, 10, 8);
                    printString("Shell Test Command Ran Successfully", 10, 8);
                    printString(charBuffer, 10 + (14 * 8), 1080 - 16);
                    charBufferIndex = 0;
                } else {
                    for (int i = 0; i < 220 - 1; ++i) {
                        charBuffer[i] = ' ';
                    }
                    printString(charBuffer, 10, 8);
                    printString("Invalid Shell Command", 10, 8);
                    printString(charBuffer, 10 + (14 * 8), 1080 - 16);
                    charBufferIndex = 0;
                }
            } else {
                char c = scancode_set1_ascii[ps2LastScanCode];
                if (c != 0 && charBufferIndex < 220 - 1) {
                    charBuffer[charBufferIndex++] = c;
                }
            }

            printString(charBuffer, 10 + (14 * 8), 1080 - 16);
        }

        __asm__ __volatile__("hlt");
    }
}
