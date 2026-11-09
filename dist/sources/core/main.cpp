#include <posix/spawn.hpp>
#include <vfs/vfs.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "[CORE] Server started" << std::endl;

    vfs::init();

    std::cout << "[CORE] Executing some test binaries" << std::endl;

    // TODO: add waiting on a thread to exit to the kernel instead of a fixed wait

    posix::spawn("/usr/bin/helloworld.elf");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    posix::spawn("/usr/bin/hellocxx.elf");
    std::this_thread::sleep_for(std::chrono::milliseconds(5200));

    std::cout << "[CORE] Executing meow.elf every 100ms forever" << std::endl;

    while (true) {
        posix::spawn("/usr/bin/meow.elf");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
