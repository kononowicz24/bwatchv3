#include <math.h>
const int digits[] = { 0x6f, 0x60, 0x3e, 0x7c, 0x71, 0x5d, 0x5f, 0x64, 0x7f, 0x7d};
//const int segments[] = {0x01, 0x02, 0x04, 0x08};

int hp_DPpos = 0x00;

void hp5082_display(int currTime) {
  for (int i=0; i<4; i++) {
      int digit = (currTime/(int)pow(10,3-i))%10;
      PORTA = digits[digit];
      if (hp_DPpos & (3-i)) PORTA |= 0x80;
      PORTB &= 0xF0;
      //PORTB += segments[3-i];
      PORTB = 1<<(3-i);
      _delay_ms(3);
    }
}
void hp5082_setDP(int position) {
  hp_DPpos = position;
}
