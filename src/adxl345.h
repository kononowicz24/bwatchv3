void adxl345_init() {
  Wire.beginTransmission(0x53);
  Wire.write(0x24);//reg0x24
  Wire.write(0x05);//0.3125g activity threshold
  Wire.write(0x04);//0.25g inactivity threshold
  Wire.write(0x0a);//10s to interrupt inactivity
  Wire.write(0xff);//both ac coupled all axis
  Wire.endTransmission();
}
