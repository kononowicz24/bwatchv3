void deepsleep_goto(int state) {
  hp5082_off();
  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
}
