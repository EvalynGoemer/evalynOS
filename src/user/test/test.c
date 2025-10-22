int main() __attribute__((section(".entry")));
int main() {
    while (1) {
        __asm__ volatile (
            "int $0x69"
        );
    }
}
