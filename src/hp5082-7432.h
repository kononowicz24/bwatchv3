#ifndef __HP5082_H
#define __HP5082_H

#include <math.h>
//const int digits[] = { 0x6f, 0x60, 0x3e, 0x7c, 0x71, 0x5d, 0x5f, 0x64, 0x7f, 0x7d};
//const int segments[] = {0x01, 0x02, 0x04, 0x08};
const int digits[] = { 0x6f, 0x60, 0x3d, 0x7c, 0x72, 0x5e, 0x5f, 0x68, 0x7f, 0x7e};//revC


int hp5802_DPpos = 0x00;

void hp5802_init() {
  DDRA = 0xff; //PORTA - SEGMENTS
  PORTA = 0xff; //all on for a self test
  DDRB |= 0x0F; //PORTB 0:4 - DIGITS
  PORTB |= 0x0F; // display self-test - all on
  _delay_ms(700); // debug or leave as an option
  PORTB &= 0xF0; //all off
}

void hp5082_display(int value) { //todo: leading zeroes, on/off
  for (int i=0; i<4; i++) {
      int digit = (value/(int)pow(10,3-i))%10;
      PORTA = digits[digit];
      if (i==1) PORTA |= 0x80;
      PORTB &= 0xF0;
      //PORTB += segments[3-i];
      PORTB |= 1<<(3-i);
      _delay_ms(1); //todo refactor, one can dispay numbers only here
    }
}

void hp5082_display2(int value, int offset) { //todo: leading zeroes, on/off
  PORTB &= 0xF0;
  for (int i=0+offset; i<2+offset; i++) {
      int digit = (value/(int)pow(10,3-i))%10;
      PORTA = digits[digit];
      //if (hp5802_DPpos & (3-i)) PORTA |= 0x80;
      PORTB &= 0xF0;
      //PORTB += segments[3-i];
      PORTB |= 1<<(3-i);
      _delay_ms(1); //todo refactor, one can dispay numbers only here
    }
}

void hp5082_setDP() {
  //hp5802_DPpos = position;
  PORTA |= 0x80;
  PORTB |= 0x0F;
}

void hp5082_off() {
  PORTA = 0x00;
  PORTB &= 0xF0;
}

#endif