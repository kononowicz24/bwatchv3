void adxl345_init() {
  Wire.beginTransmission(0x53);
  Wire.write(0x24);//reg 0x24
  Wire.write(0x04);//0.3125g activity threshold 05
  Wire.write(0x03);//0.25g inactivity threshold 04
  Wire.write(0x0a);//10s to interrupt inactivity 0a
  Wire.write(0xff);//both ac coupled wo z axis
  Wire.endTransmission();
  Wire.beginTransmission(0x53);
  Wire.write(0x2d); //interrupts reg
  Wire.write(0x28); //enable measure
  Wire.write(0x18); //active and inactivity interrupt
  Wire.write(0x08); //set inactivity interrupt to INT2
  Wire.endTransmission();
  Wire.beginTransmission(0x53);
  Wire.write(0x31); //data format reg
  Wire.write(0x20);
  Wire.endTransmission();
  PORTD |= 0x00011000;
  DDRD &= 0b11100111;//PD4,5 as INPUT
  PCMSK3 |= 0b00011000; //set PCINT27,28
  PCICR |= 0x08; //enable PCINT bank 3
}

void adxl345_clear_int() {
  Wire.beginTransmission(0x53);
  Wire.write(0x30);
  Wire.endTransmission(false);
  Wire.requestFrom(0x53, 1);
  Wire.read();
  //Serial.println(z11);
}

int16_t adxl345_zdata() {
  Wire.beginTransmission(0x53);
  Wire.write(0x36);
  Wire.endTransmission(false);
  Wire.requestFrom(0x53, 2);
  uint8_t z1 = Wire.read();
  uint8_t z2 = Wire.read();
  return (z2<<8)+z1;
}
