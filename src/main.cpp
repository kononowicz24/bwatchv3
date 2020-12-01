#include <Arduino.h>
#include <avr/interrupt.h>

#define SDA_PORT PORTC
#define SDA_PIN 1
#define SCL_PORT PORTC
#define SCL_PIN 0

#define I2C_TIMEOUT 10
#define I2C_FASTMODE 1

#include <SoftWire.h>

SoftWire Wire = SoftWire();

#include "hp5082-7432.h"
#include "adxl345.h"
#include "wirescanner.h"
#include "ds3231m.h"
#include "buttons.h"
#include "deepsleep.h"
#include "oled.h"
#include "hm11.h"

volatile int dHour = 9999;
int minute = 0;
int hour = 0;
volatile bool once = true;

char* RXString;
uint8_t RXStringMaxSize = 255;
bool rxStringComplete = false;
bool newRxString = false;
uint8_t RXbytes = 0;

#define NUMBER_OF_STATES 5

volatile int state = 2;
//volatile bool inactivity = false; //moved to deepsleep.h

#define SCREENONTIME 20000
volatile long long screenOffTime = 0;
volatile bool SW1Pressed = false;

#define RTCREFRESHMILLIS 500
volatile long long rtcLastMillis = -1*RTCREFRESHMILLIS;

void refreshTime() {
    dHour = ds3231m_getHours()*100+ds3231m_getMinutes();
    rtcLastMillis = millis();
}

ISR(PCINT3_vect) {
  cli();
  if (!(PIND & 0x20)) { //pd5
    if (inactivity) once = true;
    Serial.println("PCINT29 : ADXL_ACTIVITY");//todo //irq2 //pd3
    adxl345_clear_int();
    refreshTime();
    screenOffTime = millis() + SCREENONTIME;
    inactivity = 0;
    state = 2;
  }
  if (!(PIND & 0x10)) { //pd4 //test
    Serial.println("PCINT28 : INACTIVITY"); //INT1
    adxl345_clear_int();
    if  (millis()>screenOffTime)
      deepsleep_goto();
    //refreshTime();
    //screenOffTime = millis() + SCREENONTIME;
    //inactivity = 0;
  }
  if (!(PIND & 0x01)) { //pd0 - RX BT
    if (inactivity) {
      screenOffTime = millis() + SCREENONTIME;
      inactivity = 0;
      state = 2;
    }
  }
  sei();
}

ISR(PCINT2_vect) {
  cli();
  if (!(PINC & 0x80)) { //pressed - sw1
    Serial.println("SW1");
    hm11_wakeup();
    screenOffTime = millis() + SCREENONTIME;
    once = true;
    if (!inactivity) SW1Pressed = true;
  }
  if (!(PINC & 0x40)){ // pd2 //pressed - sw2
    Serial.println("SW2");
    hm11_wakeup();
    once = true;
    if (!inactivity) state++; else state = 2;
    //if (state>NUMBER_OF_STATES-1) state = 1;
    inactivity = 0;
    SW1Pressed = false;
    screenOffTime = millis() + SCREENONTIME;
  }
  sei();
}

void setup()
{
  cli();
  
  //PORTB = 0xFF;
  //PORTC = 0xFF;
  //PORTD = 0xFF; //ALL INPUTS ARE PULLUP - TODO: DISABLE FOR LOW POWER OPERATION
  Wire.begin();
  Serial.begin(9600);
  RXString = new char[RXStringMaxSize];
  adxl345_init();
  hp5802_init();
  hm11_init();
  buttons_init();
  oledInit(0x3c, 0, 0);
  oledFill(0);
  screenOffTime = millis() + SCREENONTIME;
  sei();
}

void loop()
{
  switch (state) {//switch STATE MACHINE
    case 2: {
      if (once) {
        oledInit(0x3c, 0, 0);
        oledShutdown();
        once = false;
      }
      for(RXbytes = 0; Serial.available(); RXbytes++) {
         // get the new byte:
        char inChar = (char)Serial.read();
        RXString[RXbytes] = inChar;
        rxStringComplete = true;
        newRxString = true;
      }
      if (newRxString)
        for (int i = RXbytes; i<RXStringMaxSize; i++) {
          RXString[i] = 0;
        }
      //validate RXString - characters with app name and 
      if (newRxString) {
        oledInit(0x3c, 0, 0);
        oledFill(0);
        oledWriteString(0,0,RXString,FONT_SMALL,0);
        newRxString = false;
      }
      hp5082_display(dHour); // blocking task to the end
      break;
    }
    case 1: { //debug, setup
      if (once) {
        oledInit(0x3c, 0, 0);
        oledFill(0);
        oledWriteString(0,0,"debug",FONT_SMALL,0);
        oledWriteString(0,1,"Ustawienia",FONT_SMALL,0);
        once = false;
      }
      hp5082_display(dHour);
      break;
    }
    case 3: {
      if (once) {
          hour = ds3231m_getHours();
          if (!SW1Pressed){//tylko przy zmianie trybu
            oledInit(0x3c, 0, 0);
            oledFill(0);
            oledWriteString(0,0,"    Ustaw godzine",FONT_SMALL,0);
            oledWriteString(0,1,"   SW1 = godzina++ ->",FONT_SMALL,0);
            oledWriteString(0,7,"SW2 = kolejny tryb ->",FONT_SMALL,0);
          }
          once = false;
      }
      if (SW1Pressed) {
        hour = ds3231m_getHours()+1;
        if (hour>23) hour =0;
        ds3231m_setHours(hour); //hours set
        hour = ds3231m_getHours();
        SW1Pressed = false;
      }
      hp5082_display2(hour*100, 0);
       
      
      break;
    }
    case 4: {
      if (once) {
          minute = ds3231m_getMinutes();
          if (!SW1Pressed) {
            oledInit(0x3c, 0, 0);
            oledFill(0);
            oledWriteString(0,0,"    Ustaw minute",FONT_SMALL,0);
            oledWriteString(0,1,"    SW1 = minuta++ ->",FONT_SMALL,0);
            oledWriteString(0,7,"SW2 = kolejny tryb ->",FONT_SMALL,0);
          }
          once = false;
      }
      if (SW1Pressed) {
        minute = ds3231m_getMinutes()+1;
        if (minute>59) minute=0;
        ds3231m_setMinutes(minute);
        minute = ds3231m_getMinutes();
        SW1Pressed = false;
      }
      hp5082_display2(minute, 2); //minutes set
      break;
    }
    default: state=2; break;
  }

  if (millis()>screenOffTime){
      state = 1;
      //hp5082_off();
      //oledShutdown();
      deepsleep_goto();
  }//check always

  if (millis()>rtcLastMillis+RTCREFRESHMILLIS) {
    refreshTime();
  }//check always

  if (minute>59) minute=0;
  if (hour>23) hour =0; //check always
}
