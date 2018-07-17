#define CYCLE 78
#define CYCLE_ON 2

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

bool led_blink = false;
volatile int interruptCounter;

void IRAM_ATTR led_isr() {
  portENTER_CRITICAL_ISR(&timerMux);
  if (led_blink) {
    interruptCounter++;
    if (interruptCounter > CYCLE) interruptCounter = 0;
    if (interruptCounter == 0) digitalWrite(4, HIGH);
    else if (interruptCounter == CYCLE_ON) digitalWrite(4, LOW);
  }
  else digitalWrite(4, LOW);
  portEXIT_CRITICAL_ISR(&timerMux);
}

void led_setup() {
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &led_isr, true);
  timerAlarmWrite(timer, 500, true);
}

void led_start(){
  if (led_blink) return;
  led_blink = true;
  timerAlarmEnable(timer);
}

void led_stop(){
  if (!led_blink) return;
  portENTER_CRITICAL_ISR(&timerMux);
  led_blink = false;
  timerAlarmDisable(timer);
  digitalWrite(4, LOW);
  portEXIT_CRITICAL_ISR(&timerMux);
}

bool led_running(){
  return led_blink;
}

