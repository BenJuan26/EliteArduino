#pragma once

#ifndef HIGH
#define HIGH 1
#endif

#ifndef LOW
#define LOW 0
#endif

typedef unsigned char byte;

extern bool buttonPressed[64];
class MockJoystick {
public:
    void button(byte button, bool val);
};

extern MockJoystick Joystick;

unsigned long millis();
void initializeClock();
