
#include <Arduino.h>


#define SDA_PORT PORTC
#define SDA_PIN 1
#define SCL_PORT PORTC
#define SCL_PIN 0

#define I2C_TIMEOUT 100
#define I2C_FASTMODE 1

#include <SoftWire.h>

SoftWire Wire = SoftWire();

#include "htu21d.h"
#include "bmp280.h"
#include "hp5082-7432.h"

int16_t* bmp280_temp_calib_info;
int16_t* bmp280_pres_calib_info;

void setup()
{
  Wire.begin();
  Serial.begin(9600);

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


  bmp280_temp_calib_info = new int16_t[3];
  bmp280_pres_calib_info = new int16_t[9];
  bmp280_calibration_temp(bmp280_temp_calib_info);
  for (int i=0; i<3; i++) {
    Serial.println(bmp280_temp_calib_info[i]);
  }
  Serial.println();
  bmp280_calibration_pres(bmp280_pres_calib_info);
  for (int i=0; i<9; i++) {
    Serial.println(bmp280_pres_calib_info[i]);
  }
  bmp280_init();

  DDRA = 0xff; //todo: when interfacing with atmega644 check which port is it
    PORTA = 0xff;
    DDRB |= 0x0F;
    PORTB |= 0x0F; // self-test wyswietlacza
    _delay_ms(1500);
    PORTB &= 0xF0;
}

double fTemp = 8888;
//double fHumi = -273.15;

#define DISPREFRESHMILLIS 5000
long long dispLastMillis = -1*DISPREFRESHMILLIS;

void loop()
{
  //Serial.print(htu21d_humi());
  if (millis()>dispLastMillis+DISPREFRESHMILLIS) {
    fTemp = bmp280_temp(bmp280_temp_calib_info);
    Serial.print(fTemp);
    Serial.println("degC@BMP280");
    dispLastMillis = millis();
  }
  //Serial.println(bmp280_isok());
  //Serial.print(htu21d_temp());
  //Serial.println("degC@HTU21D ");
  //Serial.println(bmp280_isok());
  //delay(5000);           // wait 5 seconds for next scan
  hp5082_display((int)(fTemp*100));
  hp5082_setDP(0x04);
}
