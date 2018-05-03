#define CYCLE 40
#define CYCLE_ON 1

bool led_blink = false;

void led_setup() {
  xTaskCreate(&blinky, "blinky", 512, NULL, 2, NULL );
}

void blinky(void *pvParameter)
{
  // Led PWM
  pinMode(4, OUTPUT);
  digitalWrite(4, 0);
  while (true) { 
    if (led_blink) {
      digitalWrite(4, HIGH);
      delay(CYCLE_ON);
      digitalWrite(4, LOW);
      delay(CYCLE - CYCLE_ON);
    }
    else {
      digitalWrite(4, LOW);
      delay(CYCLE);
    }
  }
  vTaskDelete( NULL );
}

void led_start(){
  led_blink = true;
}

void led_stop(){
  led_blink = false;
}

bool led_running(){
  return led_blink;
}

