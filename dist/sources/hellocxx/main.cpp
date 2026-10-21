#include <iostream>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <thread>

int main() {
    std::cout << "Hello from userspace w/ C++ && libcxx! " << std::endl;

    try {
        throw std::runtime_error("test");
    } catch (std::runtime_error& e) {
        std::cerr << "caught c++ exception: " << e.what() << std::endl;
    }

    std::vector<int> vector = {1, 2, 3, 4};
    vector.push_back(5);

    std::cout << "vector: { ";
    for (const int& num : vector) {
        std::cout << num << " " << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "}" << std::endl;

    return 0;
}
