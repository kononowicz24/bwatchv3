void deepsleep_goto(int state) {
  hp5082_off();
  state = 0;
  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
}
