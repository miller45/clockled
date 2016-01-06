#include <LiquidCrystal.h>

#include "Arduino.h"

#include "RTClib.h"

// github.com/Phaiax/sainsmartkeypad
#include "sainsmartkeypad.h"

#define PWMPIN1 3
#define PWMPIN2 10
#define PWMPIN3 11

#define SMODE_BRIGHT 0
#define SMODE_COLOR 1

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
SainsmartKeypad keypad(0);
RTC_Millis rtc;
// 13,12,11 3,2,1,0

int value = 0;
int xvalue = 0;
uint8_t key;
int selectcount = 0;
bool lightOn = false;

byte brightness = 50;
int rgb_colors[3] = {255, 0, 0};



byte sMode = 0; //0 = bright, 1=color

void setup() {
  rtc.begin(DateTime(__DATE__, __TIME__));

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Init...");
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);
  pinMode(PWMPIN1, OUTPUT);
  pinMode(PWMPIN2, OUTPUT);
  pinMode(PWMPIN3, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  WriteColorsPwm();
}

void loop() {
  long mill = millis();
  int m1000slc = mill % 1000;
  if (m1000slc == 0) {
    DisplayTime();
  }
  key = keypad.getKey_fastscroll();

  // Try the other possibilities:
  // (Only one at a time.
  //  fastscroll, waitrelease and periodic
  //  won't work next to each other.)

  //key = keypad.getKey_waitrelease(); // without fastscroll but usable
  //key = keypad.getKey_periodic(); // not quite usable
  //key = keypad.getKey_instant(); // not quite usable

  if (key != SAMPLE_WAIT) // Do not refresh screen every loop
  {
    switch (key)
    {
      case UP_KEY:
        if (sMode == SMODE_BRIGHT) {
          IncreaseBrightness();
        } else {
          CycleColorUp();
        }
        value++;
        DisplayParas();
        WriteColorsPwm();
        break;
      case DOWN_KEY:
        if (sMode == SMODE_BRIGHT) {
          DecreaseBrightness();
        } else {
          CycleColorDown();
        }
        value--;
        DisplayParas();
        WriteColorsPwm();
        break;
      case LEFT_KEY:
        sMode = SMODE_BRIGHT;
         DisplayParas();
        break;
      case RIGHT_KEY:
        sMode = SMODE_COLOR;
         DisplayParas();
        break;
      case SELECT_KEY:
        selectcount++;
        if (selectcount > 10) {
          selectcount = 0;
          ToggleLightOn();
          delay(1000);
        }
        break;
    }



  }
}

void WriteColorsPwm() {
  analogWrite(PWMPIN1, rgb_colors[0] / 100 * brightness);
  analogWrite(PWMPIN2, rgb_colors[1] / 100 * brightness);
  analogWrite(PWMPIN3, rgb_colors[2] / 100 * brightness);
}

void DisplayTime() {
  DateTime now = rtc.now();
  lcd.setCursor(0, 0);
  if (now.hour() < 10) {
    lcd.print(" ");
  }
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) {
    lcd.print("0");
  }
  lcd.print(now.minute());
  lcd.print(":");
  if (now.second() < 10) {
    lcd.print("0");
  }
  lcd.print(now.second());
}


void DisplayParas() {
  lcd.setCursor(0, 1);
  lcd.print("B");
  if(sMode==SMODE_BRIGHT){
     lcd.print("*");
  }else{
     lcd.print(" ");
  }
  lcd.print(brightness);
  lcd.print("  ");
}

void ToggleLightOn() {
  if (lightOn) {
    digitalWrite(10, LOW);
    lightOn = false;
  } else
  {
    digitalWrite(10, HIGH);
    lightOn = true;
  }
}


void IncreaseBrightness() {
  if (brightness < 100) {
    brightness++;
  }
}

void CycleColorUp() {
  if (rgb_colors[0] > 0) {
    rgb_colors[0] = 0;
    rgb_colors[1] = 255;
  } else if (rgb_colors[1] > 0) {
    rgb_colors[1] = 0;
    rgb_colors[2] = 255;
  } else {
    rgb_colors[0] = 255;
    rgb_colors[1] = 0;
    rgb_colors[2] = 0;
  }
}

void  DecreaseBrightness() {
  if (brightness > 0) {
    brightness--;
  }
}
void  CycleColorDown() {

  if (rgb_colors[2] > 0) {
    rgb_colors[2] = 0;
    rgb_colors[1] = 255;
  } else if (rgb_colors[1] > 0) {
    rgb_colors[1] = 0;
    rgb_colors[0] = 255;
  } else {
    rgb_colors[0] = 0;
    rgb_colors[1] = 0;
    rgb_colors[2] = 255;
  }

}

