
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

int16_t* bmp280_temp_calib_info;
int16_t* bmp280_pres_calib_info;
double fTemp = -273.15;

ISR(INT0_vect) {
  cli();
  fTemp = 99;
  sei();
}

void setup()
{
  //cli();
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
  bmp280_init();
  hp5802_init();
  //buttons_init();
  //sei();
}


//double fHumi = -273.15;

#define BMPREFRESHMILLIS 5000
long long dispLastMillis = -1*BMPREFRESHMILLIS;

void loop()
{
  if (millis()>dispLastMillis+BMPREFRESHMILLIS) {
    fTemp = bmp280_temp(bmp280_temp_calib_info);
    Serial.print(fTemp);
    Serial.println("degC@BMP280");
    dispLastMillis = millis();
    Serial.print(ds3231m_getHours());
    Serial.print(":");
    Serial.print(ds3231m_getMinutes());
    Serial.print(":");
    Serial.print(ds3231m_getSeconds());
  }
  //Serial.println(bmp280_isok());
  //Serial.print(htu21d_temp());
  //Serial.println("degC@HTU21D ");
  //Serial.println(bmp280_isok());
  //delay(5000);           // wait 5 seconds for next scan
  hp5082_display((int)(fTemp*100));
  hp5082_setDP(0x04);
}
