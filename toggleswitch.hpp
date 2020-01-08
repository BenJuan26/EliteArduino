#pragma once

#include "test/mock_arduino.hpp"

// Allow 5 seconds of lag from the device, to the game, and back to the device.
#define BUTTON_SYNC_TIME 5000

// Each button press will last about 100ms.
#define BUTTON_HOLD_TIME 100

#define BUTTON_PRESSED LOW
#define BUTTON_RELEASED HIGH

struct Toggleswitch {
  byte currState;
  byte keyState;
  byte buttonState;
  byte joyButton;
  unsigned long pressedTime;
  long flag;

  Toggleswitch() {}

  Toggleswitch(const byte _state, const byte _button, const long _flag) {
    currState = _state;
    keyState = _state;
    buttonState = BUTTON_RELEASED;
    joyButton = _button;
    pressedTime = 0L;
    flag = _flag;
  }

  void update(const unsigned long currentFlags, const bool flagsValid) {
    if (currState != keyState || !isInSync(currentFlags, flagsValid)) {
      keyState = currState;
      buttonState = BUTTON_PRESSED;
      Joystick.button(joyButton, true);
      pressedTime = millis();
    } else if (buttonState == BUTTON_PRESSED && millis() - pressedTime >= BUTTON_HOLD_TIME) {
      buttonState = BUTTON_RELEASED;
      Joystick.button(joyButton, false);
    }
  }

  bool isInSync(const unsigned long currentFlags, const bool flagsValid) const {
    // Always treat as in sync if there's no flag info
    if (!flagsValid) {
      return true;
    }

    bool buttonPressed = keyState == BUTTON_PRESSED;
    bool flagSet = currentFlags & flag;
    unsigned long timeSincePressed = millis() - pressedTime;

    return timeSincePressed < BUTTON_SYNC_TIME || buttonPressed == flagSet;
  }
};
