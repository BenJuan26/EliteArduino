#include "mock_arduino.hpp"
#include <iostream>
#include <chrono>

bool buttonPressed[64];
void MockJoystick::button(byte button, bool val) {
    if (val) {
        printf("Button %d pressed\n", button);
    } else {
        printf("Button %d released\n", button);
    }
    
    buttonPressed[button] = val;
}

MockJoystick Joystick;

u_int64_t currentTimeMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

u_int64_t start;
unsigned long millis() {
    u_int64_t now = currentTimeMs();
    unsigned long millis = now - start;
    // printf("Reported millis is %lu\n", millis);

    return millis;
}

void initializeClock() {
    start = currentTimeMs();
}
