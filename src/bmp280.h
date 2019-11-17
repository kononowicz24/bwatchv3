bool bmp280_isok();

void bmp280_init() {
  Wire.beginTransmission(0x76);
  Wire.write(0xF4);
  Wire.write(0x3F); //T=010 P=111 M=11 5f
  Wire.endTransmission();
  //Serial.println(bmp280_isok());
  delay(200);
}

void bmp280_calibration_temp(int16_t* calib_info) {
    Wire.beginTransmission(0x76);
    Wire.write(0x88);
    Wire.endTransmission(false);
    Wire.requestFrom(0x76, 6);
    for (int calib_data_i = 0; calib_data_i < 3; calib_data_i++) {
      uint8_t c = Wire.read();
      uint8_t d = Wire.read();
      *(calib_info+calib_data_i) = c + (d<<8);
    }
}

void bmp280_calibration_pres(int16_t* calib_info) {
    Wire.beginTransmission(0x76);
    Wire.write(0x8E);
    Wire.endTransmission(false);
    Wire.requestFrom(0x76, 18);
    for (int calib_data_i = 0; calib_data_i < 9; calib_data_i++) {
      uint8_t c = Wire.read();
      uint8_t d = Wire.read();
      *(calib_info+calib_data_i) = c + (d<<8);
    }
}

float bmp280_temp(int16_t* calib_info) {
  //Serial.println(bmp280_isok());
  Wire.beginTransmission(0x76);
  Wire.write(0xFA);
  Wire.endTransmission(false);
  Wire.requestFrom(0x76, 3);
  uint8_t meas_HI = Wire.read();
  uint8_t meas_LO = Wire.read();
  uint8_t meas_XLO = Wire.read();
  uint32_t meas_T = ((uint32_t)meas_HI<<12) + ((uint32_t)meas_LO<<4) + ((uint32_t)meas_XLO>>4);
  float var1 = (((float)meas_T)/16384.0f - ((float)((uint16_t)calib_info[0]))/1024.0f)*((float)calib_info[1]);
  float var2 = ((((float)meas_T)/131072.0f - ((float)((uint16_t)calib_info[0]))/8192.0f)*(((float)meas_T)/131072.0f - ((float)((uint16_t)calib_info[0]))/8192.0f)) * ((float)calib_info[2]);
  return (var1+var2)/5120.0f;
}

bool bmp280_isok() {
    Wire.beginTransmission(0x76);
    Wire.write(0xF4);
    Wire.endTransmission(false);
    Wire.requestFrom(0x76, 1);
    return (bool)(Wire.read());
}
