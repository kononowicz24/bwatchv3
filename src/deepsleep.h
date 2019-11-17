void deepsleep_goto() {
  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
}
