#define FLAG_LANDING_GEAR_DOWN 0x4
#define FLAG_LIGHTS_ON 0x100
#define FLAG_CARGO_SCOOP_DEPLOYED 0x200
#define FLAG_SILENT_RUNNING 0x400

#define PIN_LANDING_GEAR 9
#define PIN_LIGHTS_ON 10
#define PIN_CARGO_SCOOP 11
#define PIN_SILENT_RUNNING 12

char buffer[4];
unsigned long flags = 0;

void setup()
{
  pinMode(PIN_LANDING_GEAR, OUTPUT);
  pinMode(PIN_LIGHTS_ON, OUTPUT);
  pinMode(PIN_CARGO_SCOOP, OUTPUT);
  pinMode(PIN_SILENT_RUNNING, OUTPUT);
  Serial.begin(9600);
  Serial.println("Ready");
}

void loop()
{
  bool newData = false;
  while (Serial.available() >= 4) {
    Serial.readBytes(buffer, 4);
    flags = 0;
    flags += buffer[0] << 24;
    flags += buffer[1] << 16;
    flags += buffer[2] << 8;
    flags += buffer[3];

    newData = true;
  }

  if (newData) {
    Serial.println(flags);
  }

  digitalWrite(PIN_LANDING_GEAR, flags & FLAG_LANDING_GEAR_DOWN ? HIGH : LOW);
  digitalWrite(PIN_LIGHTS_ON, flags & FLAG_LIGHTS_ON ? HIGH : LOW);
  digitalWrite(PIN_CARGO_SCOOP, flags & FLAG_CARGO_SCOOP_DEPLOYED ? HIGH : LOW);
  digitalWrite(PIN_SILENT_RUNNING, flags & FLAG_SILENT_RUNNING ? HIGH : LOW);

  delay(50);
}