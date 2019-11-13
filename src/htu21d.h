float htu21d_temp() {
  Wire.beginTransmission(0x40);
  Wire.write(0xF3); // for humi 0xF5
  Wire.endTransmission();
  Wire.requestFrom(0x40, 2);
  uint8_t c = Wire.read();
  uint8_t d = Wire.read(); //read 2 bytes
  Serial.println(c);
  Serial.println(d);
    //Serial.print(((c<<8)+d)>>2);         // print the character
  uint16_t temp = (c<<8)+d;
  if (temp!=0x0FFF) return temp * 2.68127441e-3 -46.85f; else return -99.97f;
}

float htu21d_humi() {
  Wire.beginTransmission(0x40);
  Wire.write(0xF5); // for humi 0xF5
  Wire.endTransmission();
  Wire.requestFrom(0x40, 2);
  uint8_t c = Wire.read();
  uint8_t d = Wire.read(); //read 2 bytes
  uint16_t humi = (c<<8)+d;
  if (humi!=0x0FFF) return humi * 1.9073486328125e-3 -6.0f; else return -99.97f;
}
