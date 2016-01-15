//#define SOFTRTC

/*
  dim_curve 'lookup table' to compensate for the nonlinearity of human vision.
  Used in the getRGB function on saturation and brightness to make 'dimming' look more natural.
  Exponential function used to create values below :
  x from 0 - 255 : y = round(pow( 2.0, x+64/40.0) - 1)
*/

const byte dim_curve[] = {
  0,   1,   1,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
  3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,   4,   4,   4,
  4,   4,   4,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   6,   6,   6,
  6,   6,   6,   6,   6,   7,   7,   7,   7,   7,   7,   7,   8,   8,   8,   8,
  8,   8,   9,   9,   9,   9,   9,   9,   10,  10,  10,  10,  10,  11,  11,  11,
  11,  11,  12,  12,  12,  12,  12,  13,  13,  13,  13,  14,  14,  14,  14,  15,
  15,  15,  16,  16,  16,  16,  17,  17,  17,  18,  18,  18,  19,  19,  19,  20,
  20,  20,  21,  21,  22,  22,  22,  23,  23,  24,  24,  25,  25,  25,  26,  26,
  27,  27,  28,  28,  29,  29,  30,  30,  31,  32,  32,  33,  33,  34,  35,  35,
  36,  36,  37,  38,  38,  39,  40,  40,  41,  42,  43,  43,  44,  45,  46,  47,
  48,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,
  63,  64,  65,  66,  68,  69,  70,  71,  73,  74,  75,  76,  78,  79,  81,  82,
  83,  85,  86,  88,  90,  91,  93,  94,  96,  98,  99,  101, 103, 105, 107, 109,
  110, 112, 114, 116, 118, 121, 123, 125, 127, 129, 132, 134, 136, 139, 141, 144,
  146, 149, 151, 154, 157, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 190,
  193, 196, 200, 203, 207, 211, 214, 218, 222, 226, 230, 234, 238, 242, 248, 255,
};

#include <LiquidCrystal.h>

#include "Arduino.h"

#include "RTClib.h"

#include <Wire.h>

#include "sainsmartkeypad.h"

#define ledPinR 3
#define ledPinG 10
#define ledPinB 11

#define SMODE_BRIGHT 0
#define SMODE_COLOR 1

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
SainsmartKeypad keypad(0);
#ifdef SOFTRTC
RTC_Millis rtc;
#else 
RTC_DS1307 rtc;
#endif
// 13,12,11 3,2,1,0

byte ah = 07;
byte am = 10;
boolean alarmdone = false;
boolean timer1Hit = false; // if timer has been triggered //will be reset at 00.00

int value = 0;
int xvalue = 0;
uint8_t key;
int selectcount = 0;
bool lightOn = false;

byte brightness = 0;
#define maxbrightness 255
int rgb_colors[3] = {255, 0, 0};
int rgb_colors_final[3] = {0, 0, 0};

int sensorVal = 0;


byte sMode = 0; //0 = bright, 1=color

void setup() {
  Serial.begin(38400);
  Wire.begin();
#if SOFTRTC  
  rtc.begin(DateTime(__DATE__, __TIME__));
#else
  rtc.begin();
  if(rtc.isrunning()){
    Serial.println("RTC IS RUNNING");
  }  
#endif    
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Init...");
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);
  pinMode(ledPinR, OUTPUT);
  pinMode(ledPinG, OUTPUT);
  pinMode(ledPinB, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
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
  if(m5000slc==0){
     i2c_scan();
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
          ExecSyncAlarmLeds();
          //   ToggleLightOn();
          delay(1000);
        }
        break;
    }


    colorDemoFade();
  }
}

void WriteColorsPwm() {

  rgb_colors_final[0] = rgb_colors[0] / 255 * dim_curve[brightness];
  rgb_colors_final[1] = rgb_colors[1] / 255 * dim_curve[brightness];
  rgb_colors_final[2] = rgb_colors[2] / 255 * dim_curve[brightness];
  if ( rgb_colors_final[0] > 255 ) rgb_colors_final[0] = 255;
  if ( rgb_colors_final[1] > 255 ) rgb_colors_final[1] = 255;
  if ( rgb_colors_final[2] > 255 ) rgb_colors_final[2] = 255;
  analogWrite(ledPinR, rgb_colors_final[0] );
  analogWrite(ledPinG, rgb_colors_final[1]);
  analogWrite(ledPinB, rgb_colors_final[2]);
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

  lcd.print(" A");
  lcd.print(ah);
  lcd.print(":");
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

void  DecreaseBrightness() {
  if (brightness > 0) {
    brightness--;
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
      delay(1000 * 2);
    };

  }
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



void i2c_scan()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Unknow error at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  //delay(5000);           // wait 5 seconds for next scan
}


