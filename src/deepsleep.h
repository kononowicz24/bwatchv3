#include "oled.h"

void deepsleep_goto(int state) {
  hp5082_off();
  oledShutdown();
  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
}
