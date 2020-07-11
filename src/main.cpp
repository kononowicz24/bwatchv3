
#include <Arduino.h>
#include <avr/interrupt.h>

#define SDA_PORT PORTC
#define SDA_PIN 1
#define SCL_PORT PORTC
#define SCL_PIN 0

#define I2C_TIMEOUT 100
#define I2C_FASTMODE 1

#include <SoftWire.h>

SoftWire Wire = SoftWire();

//#include "htu21d.h"
#include "bmp280.h"
#include "hp5082-7432.h"
#include "adxl345.h"
#include "wirescanner.h"
#include "ds3231m.h"
#include "buttons.h"
#include "deepsleep.h"
#include "oled.h"

int16_t* bmp280_temp_calib_info;
int16_t* bmp280_pres_calib_info;
double fTemp = -273.15;
volatile int dHour = 9999;
int minute = 0;
int hour = 0;
volatile bool once = true;

#define NUMBER_OF_STATES 5

volatile int state = 1;
volatile bool inactivity = false;

#define SCREENONTIME 10000
volatile long long screenOffTime = 0;
volatile bool BTN3Pressed = false;

#define BMPREFRESHMILLIS 5000
long long dispLastMillis = -1*BMPREFRESHMILLIS;

#define RTCREFRESHMILLIS 1000
volatile long long rtcLastMillis = -1*RTCREFRESHMILLIS;

void refreshTime() {
    dHour = ds3231m_getHours()*100+ds3231m_getMinutes();
    rtcLastMillis = millis();
}

ISR(PCINT3_vect) {
  cli();
  if (!(PIND & 0x04)){ //btn2 pressed
    //Serial.println("PCINT26");
    once = true;
    if (!inactivity) state++; else state = 1;
    if (state>NUMBER_OF_STATES-1) state = 0;
    inactivity = 0;
    BTN3Pressed = false;
    screenOffTime = millis() + SCREENONTIME;
    
  }
 // if (!(PIND & 0x08)) {
 //   Serial.println("PCINT27 : ADXL_INACTIVITY");//todo
 //   adxl345_clear_int();
 //   screenOffTime = millis() + SCREENONTIME;
 //   inactivity = 0;
 //   
 // }
  if (!(PIND & 0x10)) {
    if (inactivity) once = true;
    //Serial.println("PCINT28 : ADXL_ACTIVITY");
    
    inactivity = 0;
    refreshTime();
    screenOffTime = millis() + SCREENONTIME;
    adxl345_clear_int();
  }
  sei();
}

ISR(PCINT2_vect) {
  cli();
  if (!(PINC & 0x10)) { //btn3 pressed
    //Serial.println("PCINT20");
    screenOffTime = millis() + SCREENONTIME;
    //inactivity = 0;
    once = true;
    if (!inactivity) BTN3Pressed = true;
  } //if PCINT2 is triggered and PC4 low
  sei();
}

void setup()
{
  cli();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //PORTB = 0xFF;
  //PORTC = 0xFF;
  //PORTD = 0xFF; //ALL INPUTS ARE PULLUP - TODO: DISABLE FOR LOW POWER OPERATION
  Wire.begin();
  Serial.begin(115200);

  //wirescanner_scan();

  bmp280_temp_calib_info = new int16_t[3];
  bmp280_pres_calib_info = new int16_t[9];
  bmp280_calibration_temp(bmp280_temp_calib_info);

  adxl345_init();
  bmp280_init();
  hp5802_init();
  buttons_init();
  oledInit(0x3c, 0, 0);
  oledFill(0);
  screenOffTime = millis() + SCREENONTIME;
  sei();
}


void loop()
{
  if (millis()>screenOffTime){
    state = 1;
    deepsleep_goto(state);
  }
  if (millis()>dispLastMillis+BMPREFRESHMILLIS) {
    if (bmp280_isok()) {
      fTemp = bmp280_temp(bmp280_temp_calib_info);
    } else Serial.println("BMP280 I2C error");
    dispLastMillis = millis();
  }
  if (millis()>rtcLastMillis+RTCREFRESHMILLIS) {
    refreshTime();
  }

  if (minute>59) minute=0;
  if (hour>23) hour =0;

  switch (state) {
    case 0: { 
      hp5082_display((int)(fTemp*100));
      break;
    }
    case 1: {
      hp5082_display(dHour);
      break;
    }
    case 2: { 
      if (once) {
        oledInit(0x3c, 0, 0);
        oledFill(0);
        oledWriteString(0,0,"HUJ",FONT_SMALL,0);
        oledWriteString(0,1,"HUJ",FONT_SMALL,0);
        oledWriteString(0,2,"1-Lines",FONT_SMALL,0);
        oledWriteString(0,3,"2-8x8 characters",FONT_SMALL,0);
        oledWriteString(0,4,"5-8x8 characters",FONT_SMALL,0);
        oledWriteString(0,5,"XdxdxDXDXdxdxDXDXdxdxDXDXd",FONT_SMALL,0);
        oledWriteString(0,6,"dupadupadupadupadupa",FONT_SMALL,0);
        oledWriteString(0,7,"1234567890abcdefghijk",FONT_SMALL,0);
        once = false;
      }
      hp5082_display(dHour);
      break;
    }
    case 3: {
      if (once) {
          hour = ds3231m_getHours();
          oledShutdown();
          once = false;
      }
      if (BTN3Pressed) {
        //int hour = ds3231m_getHours()+1;
        BTN3Pressed = false;
        hour++;
        ds3231m_setHours(hour); //hours set
      }
      hp5082_display2(hour*100, 0);
       
      
      break;
    }
    case 4: {
      if (once) {
          minute = ds3231m_getMinutes();
          oledShutdown();
          once = false;
      }
      if (BTN3Pressed) {
        //int minute = ds3231m_getMinutes()+1;
        minute++;
        ds3231m_setMinutes(minute);
        BTN3Pressed = false;
      }
      hp5082_display2(minute, 2); //minutes set
      break;
    }
  }
}
