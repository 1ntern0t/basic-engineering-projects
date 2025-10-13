#define RED_PIN     2
#define GREEN_PIN   3
#define BLUE_PIN    4
#define COMMON_ANODE  0 // 0=common cathode, 1=common anode

#define PIR_PIN     8

const unsigned long BLINK_MS = 120;

#define DEBUG_EDGES  1

struct RGB { uint8_t r, g, b; };
const RGB COL_OFF    = {  0,   0,   0};
const RGB COL_WHITE  = {255, 255, 255};
const RGB COL_PURPLE = {255,   0, 255};

bool manualHold = false; //true => ignore sensor
bool manualOn   = false; //when manualHold, true=white, false=black

inline uint8_t inv(uint8_t v) {
  return COMMON_ANODE ? (uint8_t)(255 - v) : v;
}
inline void writeRGB(uint8_t r, uint8_t g, uint8_t b) {
  analogWrite(RED_PIN,   inv(r));
  analogWrite(GREEN_PIN, inv(g));
  analogWrite(BLUE_PIN,  inv(b));
}
inline void writeRGB(const RGB &c) { writeRGB(c.r, c.g, c.b); }

inline void blinkColor(const RGB &c) {
  unsigned long phase = (millis() / BLINK_MS) & 1;
  writeRGB(phase ? c : COL_OFF);
}

#if DEBUG_EDGES
bool lastPir = LOW;
unsigned long lastChange = 0;
#endif

void setup() {
  pinMode(RED_PIN,   OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN,  OUTPUT);
  writeRGB(COL_OFF);

  pinMode(PIR_PIN, INPUT); //HCSR501 drives the line no pullup needed

  Serial.begin(9600);
  delay(20);
  Serial.println(F("\nAUTO: PIR HIGH -> blink PURPLE; PIR LOW -> WHITE"));
  Serial.println(F("Keys: '1' manual WHITE, '0' manual OFF, any other -> AUTO"));

#if DEBUG_EDGES
  lastPir = digitalRead(PIR_PIN);
  lastChange = millis();
  Serial.print(F("Boot PIR state: "));
  Serial.println(lastPir ? F("HIGH") : F("LOW"));
#endif
}

void loop() {
  if (manualHold) {
    writeRGB(manualOn ? COL_WHITE : COL_OFF);
  } else {
    int v = digitalRead(PIR_PIN);
    if (v == HIGH) {
      blinkColor(COL_PURPLE);
    } else {
      writeRGB(COL_WHITE);
    }

#if DEBUG_EDGES
    if (v != lastPir) {
      unsigned long now = millis();
      unsigned long dt = now - lastChange;
      lastChange = now;
      lastPir = v;
      if (v) {
        Serial.println(F("PIR RISE -> MOTION (HIGH)"));
      } else {
        Serial.print(F("PIR FALL -> NO_MOTION (LOW), HIGH duration="));
        Serial.print(dt); Serial.println(F("ms"));
      }
    }
#endif
  }

  if (Serial.available()) {
    int ch = Serial.read();
    if (ch == '1') {
      manualHold = true;  manualOn = true;
      writeRGB(COL_WHITE);
      Serial.println(F("Manual: WHITE ON (sensor ignored)"));
    } else if (ch == '0') {
      manualHold = true;  manualOn = false;
      writeRGB(COL_OFF);
      Serial.println(F("Manual: OFF/BLACK (sensor ignored)"));
    } else {
      manualHold = false;
      Serial.println(F("Auto mode (sensor active)"));
    }
  }
}
