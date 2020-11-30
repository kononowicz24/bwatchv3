#include "oled.h"
#include "hp5082-7432.h"
#include "hm11.h"
#include <avr/sleep.h>

volatile bool inactivity = false;

void deepsleep_goto() {
  hp5082_off();
  oledShutdown();
  hm11_sleep();
  inactivity = 1;
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sei();
  sleep_enable();
  sleep_mode();//sleep_cpu();
  sleep_disable();
  power_all_enable();
}
