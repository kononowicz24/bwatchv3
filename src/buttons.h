void buttons_init() {
/*  //BTN MODE, sleep wakeup, change mode
  DDRD &= 0xfb; // PD2 as input
  EICRA &= 0xfc;
  EICRA = 0x02; //INT0 triggered as falling edge
  EIMSK &= 0xfe;
  EIMSK = 0x01; //enable INT0
*/
  DDRD &= 0xfb;//PD2 as INPUT
  PCMSK3 |= 0x04; //set PCINT26
  PCICR |=0x08; //enable PCINT bank 3
  //BTN ++, in mode change parameter
  DDRC &= 0xef; //PC4 as input
  PCMSK2 |= 0x10; // set PCINT20
  PCICR |= 0x04; // enable PCINT bank 2
}
