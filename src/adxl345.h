void adxl345_init() {
  Wire.beginTransmission(0x53);
  Wire.write(0x24);//reg 0x24
  Wire.write(0x04);//0.5g activity threshold 
  Wire.write(0x03);//0.25g inactivity threshold
  Wire.write(0x14);//20s to interrupt inactivity 14hex
  Wire.write(0xff);//both ac coupled
  Wire.endTransmission();

  Wire.beginTransmission(0x53);
  Wire.write(0x2d); //interrupts reg
  Wire.write(0x28); //enable measure
  Wire.write(0x18); //activity and double tap interrupt
  Wire.write(0x08); //set double tap interrupt to INT2
  Wire.endTransmission();

  Wire.beginTransmission(0x53);
  Wire.write(0x31); //data format reg
  Wire.write(0x20); //interrupts active low
  Wire.endTransmission();

  PORTD |= 0x00110000;
  DDRD &= 0b11001111;//PD4,5 as INPUT
  PCMSK3 |= 0b00110000; //set PCINT29,28
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
