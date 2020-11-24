#include "oled.h"
#include "hp5082-7432.h"
#include <avr/sleep.h>

void deepsleep_goto(int state) {
  hp5082_off();
  oledShutdown();
  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
}
