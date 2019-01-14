#include <ArduinoJson.h>
#include <LiquidCrystal.h>
#include <Key.h>
#include <Keypad.h>

#define SHIFTPWM_NOSPI
const int ShiftPWM_dataPin = 17;
const int ShiftPWM_clockPin = 19;
const int ShiftPWM_latchPin = 18;

const int pwmFrequency = 75;
const int pwmMaxBrightness = 255;
const int pwmLowBrightness = 8;

const bool ShiftPWM_invertOutputs = false;
const bool ShiftPWM_balanceLoad = false;

#include <ShiftPWM.h>

#define LINE_LENGTH 16

#define BUTTON_SYNC_TIME 5000
#define BUTTON_HOLD_TIME 100
#define BUTTON_PRESSED LOW
#define BUTTON_RELEASED HIGH

#define PIPS_SYNC_TIME 2000

#define FLAG_LANDING_GEAR   0x00000004
#define FLAG_HARDPOINTS     0x00000040
#define FLAG_SHIP_LIGHTS    0x00000100
#define FLAG_CARGO_SCOOP    0x00000200
#define FLAG_SILENT_RUNNING 0x00000400
#define FLAG_NIGHT_VISION   0x10000000

long currentFlags = -1;
char lastSystem[32];
LiquidCrystal lcd(8, 13, 9, 4, 5, 6, 7);

byte pips[3];
unsigned long pipsLocalLastUpdated = 0;

const int numSwitches = 6;
Toggleswitch switches[numSwitches];
const int switchIndexStart = rows * cols - numSwitches;
const int pipsIndexStart = switchIndexStart - 4;

const byte rows = 5;
const byte cols = 5;
char keys[rows][cols] = {
  {'A', 'B', 'C', 'D', 'E'},
  {'F', 'G', 'H', 'I', 'J'},
  {'K', 'L', 'M', 'N', 'O'},
  {'P', 'Q', 'R', 'S', 'T'},
  {'U', 'V', 'W', 'X', 'Y'}
};
const int keyOffset = keys[0][0];
byte rowPins[rows] = {8, 9, 10, 11, 12};
byte colPins[cols] = {14, 15, 16, 20, 21};
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

struct Toggleswitch {
  byte currState;
  byte keyState;
  byte buttonState;
  byte joyButton;
  unsigned long releaseTime;
  long flag;

  Toggleswitch() {}

  Toggleswitch(byte state, byte button, long theFlag) {
    currState = state;
    keyState = state;
    buttonState = BUTTON_RELEASED;
    joyButton = button;
    releaseTime = 0L;
    flag = theFlag;
  }

  void update() {
    if (currState != keyState || !isInSync()) {
      keyState = currState;
      buttonState = BUTTON_PRESSED;
      Joystick.button(joyButton, true);
      releaseTime = millis() + BUTTON_HOLD_TIME;
    } else if (buttonState == BUTTON_PRESSED && millis() >= releaseTime) {
      buttonState = BUTTON_RELEASED;
      Joystick.button(joyButton, false);
    }
  }

  bool isInSync() {
    // Always treat as in sync if there's no flag info
    if (currentFlags < 0) {
      return true;
    }

    bool buttonPressed = keyState == BUTTON_PRESSED;
    bool flagSet = currentFlags & flag;
    unsigned long timeSinceReleased = abs(signed(releaseTime - millis()));

    return timeSinceReleased < BUTTON_SYNC_TIME || buttonPressed == flagSet;
  }
};

void padStringForLcd(char *dest1, char *dest2, const char *src) {
  if (strlen(src) <= LINE_LENGTH) {
    strncpy(dest1, src, strlen(src));
    for (int i = strlen(src); i < LINE_LENGTH; i++) {
      dest1[i] = ' ';
    }
    for (int i = 0; i < LINE_LENGTH; i++) {
      dest2[i] = ' ';
    }
    dest1[LINE_LENGTH] = '\0';
    dest2[LINE_LENGTH] = '\0';
    return;
  }
  
  int space = -1;
  int start = min(strlen(src), (unsigned)(LINE_LENGTH - 1));
  for (int i = start; i >= 0; i--) {
    if (src[i] == ' ') {
      space = i;
      break;
    }
  }

  if (strlen(src) > LINE_LENGTH*2) {
    // Total length is longer than the screen
    strncpy(dest1, src, LINE_LENGTH);
    strncpy(dest2, src + LINE_LENGTH, LINE_LENGTH-2);
    dest2[LINE_LENGTH-2] = '.';
    dest2[LINE_LENGTH-1] = '.';
  } else if (space >= 0 && strlen(src+space) <= LINE_LENGTH) {
    // Can break it up and the second line won't over flow
    strncpy(dest1, src, space);
    strncpy(dest2, src + space + 1, LINE_LENGTH);

    // Padding
    for (int i = space; i < LINE_LENGTH; i++) {
      dest1[i] = ' ';
    }
    for (int i = strlen(dest2); i < LINE_LENGTH; i++) {
      dest2[i] = ' ';
    }
  } else {
    // Breaking it up would overflow the second line
    strncpy(dest1, src, LINE_LENGTH);
    strncpy(dest2, src + LINE_LENGTH, LINE_LENGTH);
    for (int i = strlen(src + LINE_LENGTH); i < LINE_LENGTH; i++) {
      dest2[i] = ' ';
    }
  }

  // Null terminators
  dest1[LINE_LENGTH] = '\0';
  dest2[LINE_LENGTH] = '\0';
}

void lcdPrint(const char *text) {
  char line1[LINE_LENGTH + 1];
  char line2[LINE_LENGTH + 1];
  padStringForLcd(line1, line2, text);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(line1);
  delay(1);
  lcd.setCursor(0,1);
  lcd.print(line2);
  delay(1);
}

void increasePips(int index) {
  int i1 = (index+1) % 3;
  int i2 = (index+2) % 3;

  if (pips[index] == 8) {
    return;
  }

  if (pips[index] < 7) {
    pips[index] += 2;
    if (pips[i1] > 0) {
      pips[i1] -= 1;
    } else {
      pips[i2] -= 1;
    }
    if (pips[i2] > 0) {
      pips[i2] -= 1;
    } else {
      pips[i1] -= 1;
    }
  } else {
    pips[index] += 1;
    if (pips[i1] % 2 == 1) {
      pips[i1] -= 1;
    } else {
      pips[i2] -= 1;
    }
  }

  pipsLocalLastUpdated = millis();
  displayPips();
}

void resetPips() {
  for (int i = 0; i < 3; i++) {
    pips[i] = 4;
  }

  pipsLocalLastUpdated = millis();
  displayPips();
}

void setup()
{
  Serial.begin(9600);
  ShiftPWM.SetAmountOfRegisters(2);
  ShiftPWM.Start(pwmFrequency, pwmMaxBrightness);

  pinMode(1, INPUT_PULLUP);

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

  resetPips();

  int flags[6] = {FLAG_HARDPOINTS, FLAG_LANDING_GEAR, FLAG_CARGO_SCOOP,
    FLAG_SHIP_LIGHTS, FLAG_NIGHT_VISION, FLAG_SILENT_RUNNING};
  for (int i = 0; i < numSwitches; i++) {
    switches[i] = Toggleswitch(initialStates[i], i + switchIndexStart, flags[i]);
  }
}

void serialRx() {
  if (Serial.available()) {
    DynamicJsonBuffer jsonBuffer(512);
    JsonObject &root = jsonBuffer.parseObject(Serial);
    if (root.success()) {
      long flags = root["Flags"];
      currentFlags = flags;

      const char *starsys = root["StarSystem"];
      if (strcmp(lastSystem, starsys) != 0) {
        strcpy(lastSystem, starsys);
        char withPrefix[LINE_LENGTH * 2 + 1];
        strcpy(withPrefix, "Sys: ");
        strcat(withPrefix, starsys);
  
        lcdPrint(withPrefix);
      }

      if (millis() - pipsLocalLastUpdated > PIPS_SYNC_TIME) {
        JsonArray &pipsJson = root["Pips"];
        bool pipsChanged = false;
        for (int i = 0; i < 3; i++) {
          if (pips[i] != pipsJson[i]) {
            pipsChanged = true;
            pips[i] = pipsJson[i];
          }
        }
        if (pipsChanged) {
          displayPips();
        }
      }
    }
  }
}

void displayPips() {
  for (int i = 0; i < 4; i++) {
    if (pips[2] >= (i+1)*2) ShiftPWM.SetOne(i, pwmMaxBrightness);
    else if (pips[2] >= (i*2)+1) ShiftPWM.SetOne(i, pwmLowBrightness);
    else ShiftPWM.SetOne(i, 0);

    if (pips[1] >= (i+1)*2) ShiftPWM.SetOne(i+4, pwmMaxBrightness);
    else if (pips[1] >= (i*2)+1) ShiftPWM.SetOne(i+4, pwmLowBrightness);
    else ShiftPWM.SetOne(i+4, 0);

    if (pips[0] >= (i+1)*2) ShiftPWM.SetOne(i+8, pwmMaxBrightness);
    else if (pips[0] >= (i*2)+1) ShiftPWM.SetOne(i+8, pwmLowBrightness);
    else ShiftPWM.SetOne(i+8, 0);
  }
}

void updateKeys() {
  if (kpd.getKeys()) {
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

      else if (keyIndex >= pipsIndexStart) {
        if (state == PRESSED) {
          int pipsIndex = keyIndex - pipsIndexStart;
          if (pipsIndex < 3) {
            increasePips(pipsIndex);
          } else {
            resetPips();
          }
          Joystick.button(keyIndex, true);
        } else if (state == RELEASED) {
          Joystick.button(keyIndex, false);
        }
      }

      else {
        if (state == PRESSED) {
          Joystick.button(keyIndex, true);
        } else if (state == RELEASED) {
          Joystick.button(keyIndex, false);
        }
      }
    }
  }
}

void loop()
{
  serialRx();
  updateKeys();

  for (int i = 0; i < numSwitches; i++) {
    switches[i].update();
  }
}
