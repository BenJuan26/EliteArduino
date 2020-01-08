#include <iostream>

#include "testcases.hpp"
#include "mock_arduino.hpp"

int main(int argc, char** argv) {
    initializeClock();

    if (buttonSyncTest()) {
        std::cout << "buttonSyncTest failed\n";
        return 1;
    }

    std::cout << "Test passed\n";
    return 0;
}