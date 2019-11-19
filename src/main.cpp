
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

int16_t* bmp280_temp_calib_info;
int16_t* bmp280_pres_calib_info;
double fTemp = -273.15;
int dHour = 9999;

#define NUMBER_OF_STATES 2

volatile int state = 1;
volatile int inactivity = 0;

#define SCREENONTIME 10000
volatile long long screenOffTime = 0;

ISR(PCINT3_vect) {
  cli();
  if (!(PIND & 0x04)){
    Serial.println("PCINT26");
    if (!inactivity) state++;
    if (state>NUMBER_OF_STATES-1) state = 0;
    screenOffTime = millis() + SCREENONTIME;
  }
  if (!(PIND & 0x08)) {
    Serial.println("PCINT27 : ADXL_ACTIVITY");//todo
    adxl345_clear_int();
    inactivity = 0;
    screenOffTime = millis() + SCREENONTIME;
  }
  if (!(PIND & 0x10)) {
    Serial.println("PCINT28 : ADXL_INACTIVITY");
    adxl345_clear_int();
    inactivity = 1;
  }
  sei();
}

ISR(PCINT2_vect) {
  cli();
  if (!(PINC & 0x10)) {
    Serial.println("PCINT20");
    screenOffTime = millis() + SCREENONTIME;
    inactivity = 0;
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
  Serial.begin(9600);

  wirescanner_scan();

  bmp280_temp_calib_info = new int16_t[3];
  bmp280_pres_calib_info = new int16_t[9];
  bmp280_calibration_temp(bmp280_temp_calib_info);
  /*for (int i=0; i<3; i++) {
    Serial.println(bmp280_temp_calib_info[i]);
  }
  Serial.println();
  bmp280_calibration_pres(bmp280_pres_calib_info);
  for (int i=0; i<9; i++) {
    Serial.println(bmp280_pres_calib_info[i]);
  }*/
  adxl345_init();
  bmp280_init();
  hp5802_init();
  buttons_init();
  screenOffTime = millis() + SCREENONTIME;
  sei();
}


//double fHumi = -273.15;

#define BMPREFRESHMILLIS 5000
long long dispLastMillis = -1*BMPREFRESHMILLIS;

#define RTCREFRESHMILLIS 1000
long long rtcLastMillis = -1*RTCREFRESHMILLIS;

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
    dHour = ds3231m_getHours()*100+ds3231m_getMinutes();
    //Serial.print(dHour);
    //Serial.println(" @RTC");
    rtcLastMillis = millis();
  }
  //Serial.println(bmp280_isok());
  //Serial.print(htu21d_temp());
  //Serial.println("degC@HTU21D ");
  //Serial.println(bmp280_isok());
  //delay(5000);           // wait 5 seconds for next scan
  //todo: opakowac w maszyne stanow
  switch (state) {
    case 0: hp5082_display((int)(fTemp*100));
            break;
    case 1: hp5082_display(dHour);
  }

  //if (inactivity==1)

}
