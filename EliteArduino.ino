#include <ArduinoJson.h>
#include <Key.h>
#include <Keypad.h>

#include "toggleswitch.hpp"
#include "lcd.hpp"
#include "flags.hpp"

#define UPDATE_STALE_TIME 12000

#define BUTTON_LED_PIN 3

unsigned long currentFlags = 0;
unsigned long lastUpdate = 0;
bool flagsValid = false;

char lastSystem[32];

const byte rows = 4;
const byte cols = 5;
char keys[rows][cols] = {
  {'A', 'B', 'C', 'D', 'E'},
  {'F', 'G', 'H', 'I', 'J'},
  {'K', 'L', 'M', 'N', 'O'},
  {'P', 'Q', 'R', 'S', 'T'}
};

// The ASCII value of the first button in the matrix.
// This will allow any key code to be turned into a zero-based
// index for use with arrays.
const int keyOffset = keys[0][0];

byte rowPins[rows] = {14, 15, 16, 17};
byte colPins[cols] = {19, 20, 21, 22, 23};
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

const int numSwitches = 7;

Toggleswitch switches[numSwitches];
const int switchIndexStart = rows * cols - numSwitches;

void setup()
{
  Serial.begin(9600);

  pinMode(1, INPUT_PULLUP);
  pinMode(BUTTON_LED_PIN, OUTPUT);
  digitalWrite(BUTTON_LED_PIN, HIGH);

  Joystick.X(512);
  Joystick.Y(512);
  Joystick.Z(512);
  Joystick.Zrotate(512);
  Joystick.sliderLeft(512);
  Joystick.sliderRight(512);
  Joystick.hat(-1);

  lcd.clear();
  lcd.begin(16, 2);

  lcdPrint("Waiting for connection...");

  byte initialStates[numSwitches];
  for (int i = 0; i < numSwitches; i++) {
    initialStates[i] = BUTTON_RELEASED;
  }

  if (kpd.getKeys()) {
    for (int i = 0; i < LIST_MAX; i++) {
      int keyIndex = (int)kpd.key[i].kchar - keyOffset;
      if (keyIndex >= switchIndexStart) {
        int switchIndex = keyIndex - switchIndexStart;
        KeyState state = kpd.key[i].kstate;
        if (state == PRESSED || state == HOLD) {
          initialStates[switchIndex] = BUTTON_PRESSED;
        }
      }
    }
  }

  int flags[numSwitches] = {FLAG_HARDPOINTS, FLAG_LANDING_GEAR, FLAG_CARGO_SCOOP,
    FLAG_SHIP_LIGHTS, FLAG_NIGHT_VISION, FLAG_SRV_TURRET_MODE, FLAG_SILENT_RUNNING};
  for (int i = 0; i < numSwitches; i++) {
    switches[i] = Toggleswitch(initialStates[i], i + switchIndexStart + 1, flags[i]);
  }
}

void serialRx() {
  if (!Serial.available()) {
    unsigned long currentTime = millis();
    if (flagsValid && currentTime - lastUpdate > UPDATE_STALE_TIME) {
      flagsValid = false;
      lcdPrint("Waiting for connection...");
    }
    return;
  }

  DynamicJsonBuffer jsonBuffer(512);
  JsonObject &root = jsonBuffer.parseObject(Serial);
  if (!root.success()) {
    return;
  }

  long flags = root["Flags"];
  currentFlags = flags;

  const char *starsys = root["StarSystem"];
  if (!flagsValid || strcmp(lastSystem, starsys) != 0) {
    strcpy(lastSystem, starsys);
    char withPrefix[LINE_LENGTH * 2 + 1];
    strcpy(withPrefix, "Sys: ");
    strcat(withPrefix, starsys);

    lcdPrint(withPrefix);
  }

  flagsValid = true;
  lastUpdate = millis();
}

void updateKeys() {
  if (!kpd.getKeys()) {
    return;
  }

  for (int i = 0; i < LIST_MAX; i++) {
    if (kpd.key[i].kchar == NO_KEY) {
      continue;
    }
    int keyIndex = (int)kpd.key[i].kchar - keyOffset;
    KeyState state = kpd.key[i].kstate;

    if (keyIndex >= switchIndexStart) {
      int switchIndex = keyIndex - switchIndexStart;
      if (state == PRESSED) {
        switches[switchIndex].currState = BUTTON_PRESSED;
      } else if (state == RELEASED) {
        switches[switchIndex].currState = BUTTON_RELEASED;
      }
    }

    else {
      if (state == PRESSED) {
        Joystick.button(keyIndex+1, true);
      } else if (state == RELEASED) {
        Joystick.button(keyIndex+1, false);
      }
    }
  }
}

void loop()
{
  serialRx();
  updateKeys();

  for (int i = 0; i < numSwitches; i++) {
    switches[i].update(currentFlags, flagsValid);
  }
}
