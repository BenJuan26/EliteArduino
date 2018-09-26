#include <ArduinoJson.h>

#define BUTTON_SYNC_TIME 5000
#define BUTTON_HOLD_TIME 100
#define BUTTON_PRESSED LOW
#define BUTTON_RELEASED HIGH

#define FLAG_TEST 0x1

long currentFlags = -1;

struct Toggleswitch {
  byte pin;
  byte pinState;
  byte buttonState;
  byte outputPin;
  unsigned long releaseTime;
  long flag;

  void update() {
    int currState = digitalRead(pin);
    if (currState != pinState || !isInSync()) {
      pinState = currState;
      buttonState = BUTTON_PRESSED;
      digitalWrite(outputPin, HIGH);
      releaseTime = millis() + BUTTON_HOLD_TIME;
    } else if (buttonState == BUTTON_PRESSED && millis() >= releaseTime) {
      buttonState = BUTTON_RELEASED;
      digitalWrite(outputPin, LOW);
    }
  }

  bool isInSync() {
    // Always treat as in sync if there's no flag info
    if (currentFlags < 0) {
      return true;
    }

    bool buttonPressed = pinState == BUTTON_PRESSED;
    bool flagSet = currentFlags & flag;
    unsigned long timeSinceReleased = abs(signed(releaseTime - millis()));

    return timeSinceReleased < BUTTON_SYNC_TIME || buttonPressed == flagSet;
  }
};

int totalSwitches = 0;
Toggleswitch switches[5];

void setup()
{
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(1, INPUT_PULLUP);

  // Give time for digitalRead() to be accurate
  delay(1);
  byte initialState = digitalRead(1);
  switches[0] = Toggleswitch{1, initialState, initialState, 13, 0, FLAG_TEST};
  totalSwitches++;
}

void loop()
{
  if (Serial.available()) {
    DynamicJsonBuffer jsonBuffer(512);
    JsonObject &root = jsonBuffer.parseObject(Serial);
    if (root.success()) {
      long flags = root["Flags"];
      currentFlags = flags;
    }
  }

  for (int i = 0; i < totalSwitches; i++) {
    switches[i].update();
  }
  delay(20);
}