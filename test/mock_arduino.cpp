#include "mock_arduino.hpp"

#include <iostream>
#include <chrono>

typedef unsigned long long uint64;

bool buttonPressed[64];
void usb_joystick_class::button(byte button, bool val) {
    if (val) {
        printf("Button %d pressed\n", button);
    } else {
        printf("Button %d released\n", button);
    }
    
    buttonPressed[button] = val;
}

usb_joystick_class Joystick;

uint64 currentTimeMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

uint64 start;
unsigned long millis() {
    uint64 now = currentTimeMs();
    unsigned long millis = now - start;
    // printf("Reported millis is %lu\n", millis);

    return millis;
}

void initializeClock() {
    start = currentTimeMs();
}

void addTime(unsigned long ms) {
    start -= ms;
}
