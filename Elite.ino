#include <ArduinoJson.h>

#define ENG_PIN_1 9
#define ENG_PIN_2 10
#define ENG_PIN_3 11
#define ENG_PIN_4 12

void setup() {
  pinMode(ENG_PIN_1, OUTPUT);
  pinMode(ENG_PIN_2, OUTPUT);
  pinMode(ENG_PIN_3, OUTPUT);
  pinMode(ENG_PIN_4, OUTPUT);

  Serial.begin(9600);
  Serial.println("Ready");
}

void loop()
{
  if (Serial.available()) {
    DynamicJsonBuffer jsonBuffer(512);
    JsonObject &root = jsonBuffer.parseObject(Serial);
    if (root.success()) {
      JsonArray &pips = root["Pips"];
      int eng = pips[1];
      digitalWrite(ENG_PIN_1, eng > 1 ? HIGH : LOW);
      digitalWrite(ENG_PIN_2, eng > 3 ? HIGH : LOW);
      digitalWrite(ENG_PIN_3, eng > 5 ? HIGH : LOW);
      digitalWrite(ENG_PIN_4, eng > 7 ? HIGH : LOW);
    }
  }
  delay(10);
}
