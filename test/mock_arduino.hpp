#pragma once

#ifndef ARDUINO

#ifndef HIGH
#define HIGH 1
#endif

#ifndef LOW
#define LOW 0
#endif

typedef unsigned char byte;

extern bool buttonPressed[64];
class usb_joystick_class {
public:
    void button(byte button, bool val);
};

extern usb_joystick_class Joystick;

unsigned long millis();
void initializeClock();
void addTime(unsigned long ms);

#endif // ARDUINO