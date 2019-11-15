void buttons_init() {
  DDRD &= 0xfb; // PD2 as input
  EICRA &= 0xfc;
  EICRA = 0x02; //INT0 triggered as falling edge
  EIMSK &= 0xfe;
  EIMSK = 0x01; //trigger INT0
}
