uint8_t ds3231m_getSeconds() {
  Wire.beginTransmission(0x68);
  Wire.write(0x00);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 1);
  uint8_t seconds = Wire.read();
  return (seconds>>4)*10+(seconds&0x0F);
}

uint8_t ds3231m_getMinutes() {
  Wire.beginTransmission(0x68);
  Wire.write(0x01);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 1);
  uint8_t minutes = Wire.read();
  return (minutes>>4)*10+(minutes&0x0F);
}

uint8_t ds3231m_getHours() {
  Wire.beginTransmission(0x68);
  Wire.write(0x02);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 1);
  uint8_t hours = Wire.read();
  return ((hours&0x3f)>>4)*10+(hours&0x0F);
}

void ds3231m_setSeconds(int seconds) {
  Wire.beginTransmission(0x68);
  Wire.write(0x00);
  Wire.write(seconds%10+(((seconds/10)<<4)&0x70));
  Wire.endTransmission();
}

void ds3231m_setMinutes(int minutes) {
  Wire.beginTransmission(0x68);
  Wire.write(0x01);
  Wire.write(minutes%10+(((minutes/10)<<4)&0x70));
  Wire.endTransmission();
}

void ds3231m_setHours(int hours) {
  Wire.beginTransmission(0x68);
  Wire.write(0x02);
  //Serial.println((hours%10+(((hours/10)<<4)&0x70))&0x3f);
  Wire.write((hours%10+(((hours/10)<<4)&0x70))&0x3f);
  Wire.endTransmission();
}
