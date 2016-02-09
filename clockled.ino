#define GISDIGITAL
/*
 * Pins occp.
2, 
4, 5, 6, 7, 8, 9
3, 10, 11
11, 12, 13

Timer0 Pin 5,6
Timer1 Pin 9,10
Timer2 Pin 3,11
*/


//#define SOFTRTC

#include <LiquidCrystal.h>

#include "Arduino.h"

#include "RTClib.h"

#include <Wire.h>

#include <IRremoteSlim.h>

#include "sainsmartkeypad.h"

#include "ledcodes.h"

#define ledPinR 5
#define ledPinG 10
#define ledPinB 11

#define irPin 12

#define SMODE_BRIGHT 0
#define SMODE_COLOR 1
#define SMODE_HOUR 2
#define SMODE_MIN 3

LiquidCrystal lcd(8, 9, 4, 3, 6, 7);
SainsmartKeypad keypad(0);
#ifdef SOFTRTC
RTC_Millis rtc;
#else
RTC_DS1307 rtc;
#endif

IRrecv irrecv(irPin);

//  13,12,11 3,2

byte ah = 06;
byte am = 00;
boolean alarmdone = false;
boolean timer1Hit = false; // if timer has been triggered //will be reset at 00.00

int value = 0;
int xvalue = 0;
uint8_t key;
int selectcount = 0;
bool lightOn = false;

byte brightness = 255;
#define maxbrightness 255
int rgb_colors[3] = {255, 255, 255};
int rgb_colors_final[3] = {0, 0, 0};

boolean masterLedsOn = false; //all leds on off

int sensorVal = 0;


byte sMode = 0; //0 = bright, 1=color

void setup() {
  Serial.begin(38400);
  Wire.begin();
#if SOFTRTC
  rtc.begin(DateTime(__DATE__, __TIME__));
#else
  rtc.begin();
  if (rtc.isrunning()) {
    Serial.println("RTC IS RUNNING");
  }
#endif
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Init...");

  pinMode(ledPinR, OUTPUT);
  pinMode(ledPinG, OUTPUT);
  pinMode(ledPinB, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  irrecv.enableIRIn();  // Start the receiver
  TIMER_ENABLE_PWM;
  WriteColorsPwm();
}

void loop() {
  long mill = millis();
  int m1000slc = mill % 1000;
  int m5000slc = mill % 5000;
  if (m1000slc == 0) {
    CheckAlarm();
    DisplayTime();
  }
  if (m5000slc == 0) {
    // i2c_scan();
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
        if (sMode == SMODE_HOUR) {
          IncreaseHour();
        } 
        else 
        if(sMode==SMODE_MIN){
          IncreaseMin();
        }
        
        DisplayTime();        
        break;
      case DOWN_KEY:
        if (sMode == SMODE_HOUR) {
          DecreaseHour();
        } 
        else 
        if(sMode==SMODE_MIN){
          DecreaseMin();
        }
        
        DisplayTime();        
        break;
      case LEFT_KEY:
        sMode = SMODE_HOUR;
        DisplayParas();
        break;
      case RIGHT_KEY:
        sMode = SMODE_MIN;
        DisplayParas();
        break;
      case SELECT_KEY:
        selectcount++;
        if (selectcount > 10) {
          selectcount = 0;
          ExecSyncAlarmLeds();
          //   ToggleLightOn();
          delay(1000);
        }
        break;
    }


    //colorDemoFade();
  }

  //IR Stuff

  decode_results  results;        // Somewhere to store the results
  if (irrecv.decode(&results)) {  // Grab an IR code
    ProcessIRCode(&results);
    irrecv.resume();              // Prepare for the next value
  }


}

void WriteAllOff() {
  analogWrite(ledPinR, 0 );
  #ifdef GISDIGITAL
  digitalWrite(ledPinG, LOW );
  #else  
  analogWrite(ledPinG, 0 );
  #endif      
  analogWrite(ledPinB, 0 );
}

void WriteColorsPwm() {

  rgb_colors_final[0] = rgb_colors[0] / maxbrightness * dim_curve[brightness];
  rgb_colors_final[1] = rgb_colors[1] / maxbrightness * dim_curve[brightness];
  rgb_colors_final[2] = rgb_colors[2] / maxbrightness * dim_curve[brightness];
  if ( rgb_colors_final[0] > maxbrightness ) rgb_colors_final[0] = maxbrightness;
  if ( rgb_colors_final[1] > maxbrightness ) rgb_colors_final[1] = maxbrightness;
  if ( rgb_colors_final[2] > maxbrightness ) rgb_colors_final[2] = maxbrightness;

  analogWrite(ledPinR, rgb_colors_final[0]);   
  #ifdef GISDIGITAL
  if(rgb_colors_final[1]>170){
     digitalWrite(ledPinG,HIGH);
  }else{
     digitalWrite(ledPinG,LOW);
  }
  #else
  analogWrite(ledPinG, rgb_colors_final[1]);
  #endif
  analogWrite(ledPinB, rgb_colors_final[2]);
}

void SetRGB(int R, int G, int B) {
  rgb_colors[0] = R;
  rgb_colors[1] = G;
  rgb_colors[2] = B;
}

void DisplayTime() {
  DateTime now = rtc.now();
  lcd.setCursor(0, 0);
  if (now.hour() < 10) lcd.print(" ");
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) lcd.print("0");
  lcd.print(now.minute());
  lcd.print(":");
  if (now.second() < 10) lcd.print("0");
  lcd.print(now.second());
  lcd.print(" A");
  if(ah<10) lcd.print(" ");
  lcd.print(ah);
  lcd.print(":");
  if(am<10) lcd.print("0");
  lcd.print(am);
}


void DisplayParas() {
  lcd.setCursor(0, 1);
  lcd.print("B");
  if (sMode == SMODE_BRIGHT) {
    lcd.print("*");
  } else {
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
  if (brightness < maxbrightness) {
    brightness++;
  }
}

void IncreaseBrightnessD10() {
  if (brightness+25 < maxbrightness) {
    brightness+=25;
  }
}

void EnsureMinBrightness(){
  if(brightness==0){
     brightness=25;
  }
}

void CycleColorUp() {
#ifdef altc
  if (sensorVal < 1023) {
    sensorVal++;
  }
#else
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

#endif

}

void IncreaseHour(){
  alarmdone=false;
  if (ah > 23) {
    ah=0;
  }else{
    ah+=1;
  }
}

void DecreaseHour(){
  alarmdone=false;
  if(ah<1){
    ah=23;
  }else{
    ah-=1;
  }
}

void IncreaseMin(){
  alarmdone=false;
  if (am > 58) {
    am=0;
  }else{
    am+=1;
  }
}

void DecreaseMin(){
  alarmdone=false;
  if(am<1){
    am=59;
  }else{
    am-=1;
  }
}

void  DecreaseBrightness() {
  if (brightness > 0) {
    brightness--;
  }
}

void  DecreaseBrightnessD10() {
  if (brightness-25 > 0) {
    brightness-=25;
  }
}

void  CycleColorDown() {
#ifdef altc
  if (sensorVal > 0) {
    sensorVal--;
  }
#else
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
#endif

}


void CheckAlarm() {
  DateTime now = rtc.now();
  byte hh = now.hour();
  byte mm = now.minute();

  if (!alarmdone) {
    if (hh == ah && mm == am) {
      alarmdone = true;
      lcd.setCursor(7, 1);
      lcd.print("ALARM");
      ExecSyncAlarmLeds();
    }
  } else {
    //alarmdone amd jump
    if (hh == 0 && mm == 0) {
      alarmdone = false;
      lcd.setCursor(7, 1);
      lcd.print("     ");
    }
  };
}

void ExecSyncAlarmLeds() {
  lcd.setCursor(0, 1);
  lcd.print("T ");
  rgb_colors[0] = 255;
  rgb_colors[1] = 255;
  rgb_colors[2] = 255;
  brightness = 0;
  WriteColorsPwm();
  for (int i = 0; i < maxbrightness; i++) {
    brightness++;
    lcd.setCursor(3, 1);
    lcd.print(i);
    lcd.print(" ");
    WriteColorsPwm();
    key = keypad.getKey_instant();
    if (key != SELECT_KEY) {
      delay(100 * 2);
    };

  }
  brightness=255;
  SetRGB(255,255,255);
  WriteColorsPwm();
  if (key != SELECT_KEY) {
    key = keypad.getKey_waitrelease();
  }
  lcd.setCursor(0, 1);
  lcd.print("       ");
}

/*
  Control a RGB led with Hue, Saturation and Brightness (HSB / HSV )

  Hue is change by an analog input.
  Brightness is changed by a fading function.
  Saturation stays constant at 255

  getRGB() function based on <http://www.codeproject.com/miscctrl/CPicker.asp>
  dim_curve idea by Jims <http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1174674545>

  created 05-01-2010 by kasperkamperman.com
*/



const int sensorPin = 0;  // pin the potmeter is attached too



int fadeVal   = 0; // value that changes between 0-255
int fadeSpeed = 4; // 'speed' of fading


int hue;
int saturation;
#ifdef nix
void colorDemoFade()  {

  return;
  // fade from 0 - 255 and back with a certain speed
  fadeVal = fadeVal + fadeSpeed;         // change fadeVal by speed
  fadeVal = constrain(fadeVal, 0, 255);  // keep fadeVal between 0 and 255

  if (fadeVal == 255 || fadeVal == 0)    // change from up>down or down-up (negative/positive)
  { fadeSpeed = -fadeSpeed;
  }

  // set HSB values
  hue        = map(sensorVal, 0, 1023, 0, 359);   // hue is a number between 0 and 360
  saturation = 255;                               // saturation is a number between 0 - 255
  brightness = fadeVal;                           // value is a number between 0 - 255

  getRGB(hue, saturation, brightness, rgb_colors); // converts HSB to RGB

  analogWrite(ledPinR, rgb_colors[0]);            // red value in index 0 of rgb_colors array
  analogWrite(ledPinG, rgb_colors[1]);            // green value in index 1 of rgb_colors array
  analogWrite(ledPinB, rgb_colors[2]);            // blue value in index 2 of rgb_colors array

  delay(20); // delay to slow down fading
}

void getRGB(int hue, int sat, int val, int colors[3]) {
  /* convert hue, saturation and brightness ( HSB/HSV ) to RGB
     The dim_curve is used only on brightness/value and on saturation (inverted).
     This looks the most natural.
  */

  val = dim_curve[val];
  sat = 255 - dim_curve[255 - sat];

  int r;
  int g;
  int b;
  int base;

  if (sat == 0) { // Acromatic color (gray). Hue doesn't mind.
    colors[0] = val;
    colors[1] = val;
    colors[2] = val;
  } else  {

    base = ((255 - sat) * val) >> 8;

    switch (hue / 60) {
      case 0:
        r = val;
        g = (((val - base) * hue) / 60) + base;
        b = base;
        break;

      case 1:
        r = (((val - base) * (60 - (hue % 60))) / 60) + base;
        g = val;
        b = base;
        break;

      case 2:
        r = base;
        g = val;
        b = (((val - base) * (hue % 60)) / 60) + base;
        break;

      case 3:
        r = base;
        g = (((val - base) * (60 - (hue % 60))) / 60) + base;
        b = val;
        break;

      case 4:
        r = (((val - base) * (hue % 60)) / 60) + base;
        g = base;
        b = val;
        break;

      case 5:
        r = val;
        g = base;
        b = (((val - base) * (60 - (hue % 60))) / 60) + base;
        break;
    }

    colors[0] = r;
    colors[1] = g;
    colors[2] = b;
  }

}
#endif
//
void  ProcessIRCode (decode_results *results)
{
  if (results->decode_type == NEC) {
    switch (results->value) {
      case LE_OFF:
        WriteAllOff();
        break;
      case LE_ON:
        EnsureMinBrightness();
        WriteColorsPwm();
        break;
      case LE_W:
        SetRGB(255, 255, 255);
        EnsureMinBrightness();
        WriteColorsPwm();        
        break;
      case LE_R1:
        SetRGB(255, 0, 0);
        EnsureMinBrightness();
        WriteColorsPwm();
        break;
      case LE_G1:
        SetRGB(0, 255, 0);
        EnsureMinBrightness();
        WriteColorsPwm();
        break;
      case LE_B1:
        SetRGB(0, 0, 255);
        EnsureMinBrightness();
        WriteColorsPwm();
        break;
      case LE_DIMMER:
        DecreaseBrightnessD10();
        WriteColorsPwm();
        break;
      case LE_BRIGHTER:
        IncreaseBrightnessD10();
        WriteColorsPwm();
        break;
      case LE_FADE:
        ExecSyncAlarmLeds();
        break;
    }
  }
}


