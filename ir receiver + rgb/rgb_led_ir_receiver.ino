#include <Arduino.h>
#include <IRremote.h>

#define PIN_R 7
#define PIN_G 6
#define PIN_B 5
#define IR_PIN 11
#define COMMON_ANODE 0

#define IR_POWER 0x45
#define IR_1     0x16
#define IR_2     0x0C
#define IR_3     0x18
#define IR_4     0x5E
#define IR_5     0x08

bool g_powerOn = false;
uint8_t lastCmd = 0;
unsigned long lastCmdMs = 0;

inline uint8_t inv(uint8_t v){ return COMMON_ANODE ? (uint8_t)(255 - v) : v; }
inline void writeRGB(uint8_t r,uint8_t g,uint8_t b){
  analogWrite(PIN_R, inv(r));
  analogWrite(PIN_G, inv(g));
  analogWrite(PIN_B, inv(b));
}
inline void ledOff(){ writeRGB(0,0,0); }
inline void ledWhite(){ writeRGB(255,255,255); }

void setColorByKey(uint8_t cmd){
  if(!g_powerOn) return;
  if(cmd == IR_1) writeRGB(0,0,255);       // Blue
  else if(cmd == IR_2) writeRGB(0,255,0);  // Green
  else if(cmd == IR_3) writeRGB(255,0,255);// Purple
  else if(cmd == IR_4) writeRGB(255,255,0);// Yellow
  else if(cmd == IR_5) writeRGB(255,0,0);  // Red
}

void setup(){
  pinMode(PIN_R,OUTPUT);
  pinMode(PIN_G,OUTPUT);
  pinMode(PIN_B,OUTPUT);
  ledOff();

  Serial.begin(9600);
  delay(50);
  Serial.println(F("IR RGB Controller (1–5 colors)"));
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);
}

void loop(){
  if(IrReceiver.decode()){
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
      IrReceiver.resume();
      return;
    }

    uint8_t cmd = IrReceiver.decodedIRData.command;
    Serial.print(F("IR cmd=0x")); Serial.println(cmd, HEX);

    unsigned long now = millis();
    if (cmd == lastCmd && (now - lastCmdMs) < 120) {
      IrReceiver.resume();
      return;
    }
    lastCmd = cmd; lastCmdMs = now;

    if(cmd == IR_POWER){
      g_powerOn = !g_powerOn;
      if(g_powerOn) ledWhite(); else ledOff();
    }else{
      setColorByKey(cmd);
    }

    IrReceiver.resume();
  }
}
