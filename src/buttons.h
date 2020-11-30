void buttons_init() {

  //DDRD &= 0xfb;//PD2 as INPUT
  //PCMSK3 |= 0x04; //set PCINT26
  //PCICR |=0x08; //enable PCINT bank 3
  ////BTN ++, in mode change parameter
  //DDRC &= 0xef; //PC4 as input
  //PCMSK2 |= 0x10; // set PCINT20
  //PCICR |= 0x04; // enable PCINT bank 2
  DDRC &= 0x3F; // SET PC7,PC6 as inputs
  //PC7 = SW1 - upper
  //PC6 = SW2 - lower
  PCMSK2 |= 0xC0; //enable PCINT22,23
  PCICR |= 0x04; //enable PCINT bank 2
}
