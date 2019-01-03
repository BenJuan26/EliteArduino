#include <ArduinoJson.h>
#include <LiquidCrystal.h>

#define SHIFTPWM_NOSPI
const int ShiftPWM_dataPin = 2;
const int ShiftPWM_clockPin = 3;
const int ShiftPWM_latchPin = 4;

const int pwmFrequency = 75;
const int pwmMaxBrightness = 255;

const bool ShiftPWM_invertOutputs = false;
const bool ShiftPWM_balanceLoad = false;

#include <ShiftPWM.h>

#define LINE_LENGTH 16

#define BUTTON_SYNC_TIME 5000
#define BUTTON_HOLD_TIME 100
#define BUTTON_PRESSED LOW
#define BUTTON_RELEASED HIGH

#define PIPS_SYNC_TIME 2000

#define FLAG_CARGO_SCOOP 0x200

long currentFlags = -1;

struct Toggleswitch {
  byte pin;
  byte pinState;
  byte buttonState;
  byte joyButton;
  unsigned long releaseTime;
  long flag;

  void update() {
    int currState = digitalRead(pin);
    if (currState != pinState || !isInSync()) {
      pinState = currState;
      buttonState = BUTTON_PRESSED;
      // Joystick.button(joyButton, true);
      releaseTime = millis() + BUTTON_HOLD_TIME;
    } else if (buttonState == BUTTON_PRESSED && millis() >= releaseTime) {
      buttonState = BUTTON_RELEASED;
      // Joystick.button(joyButton, false);
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

LiquidCrystal lcd(8, 13, 9, 4, 5, 6, 7);

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

byte pips[3];
unsigned long pipsLocalLastUpdated = 0;

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
}

void resetPips() {
  for (int i = 0; i < 3; i++) {
    pips[i] = 4;
  }

  pipsLocalLastUpdated = millis();
}

int totalSwitches = 0;
Toggleswitch switches[5];
char lastSystem[32];

void setup()
{
  Serial.begin(9600);
  ShiftPWM.SetAmountOfRegisters(2);
  ShiftPWM.Start(pwmFrequency, pwmMaxBrightness);

  pinMode(1, INPUT_PULLUP);

  // Joystick.X(512);
  // Joystick.Y(512);
  // Joystick.Z(512);
  // Joystick.Zrotate(512);
  // Joystick.sliderLeft(512);
  // Joystick.sliderRight(512);
  // Joystick.hat(-1);

  lcd.clear();
  lcd.begin(16, 2);

  lcdPrint("Waiting for connection...");

  // Give time for digitalRead() to be accurate
//  delay(1);
//  byte initialState = digitalRead(1);
//  switches[0] = Toggleswitch{1, initialState, initialState, 1, 0, FLAG_CARGO_SCOOP};
//  totalSwitches++;
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

const int pwmLowBrightness = 8;

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

void loop()
{
  serialRx();

  for (int i = 0; i < totalSwitches; i++) {
    switches[i].update();
  }
}
