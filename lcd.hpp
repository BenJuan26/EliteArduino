#include <LiquidCrystal.h>

#define LINE_LENGTH 16

LiquidCrystal lcd(5, 6, 7, 8, 9, 10);

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