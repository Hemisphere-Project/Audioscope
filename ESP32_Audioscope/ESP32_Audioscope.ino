/*
   SETTINGS
*/
#define MP_VERSION  0.50   // remove OSC, disable wifi after 15s
#define MP_VERSION  0.51   // timeout RFID in ms instead of counter
#define MP_VERSION  0.52   // pin led task to CPU0
#define MP_VERSION  0.53   // pin led task to ISR
#define MP_VERSION  1.00   // WAV recode
#define MP_VERSION  1.01   // Change Gain & close led + no loop
#define MP_VERSION  1.02   // no loop
#define MP_VERSION  1.03   // gain again
#define MP_VERSION  1.04   // fix loop

#define PIN_MOTOR 15

/*
   INCLUDES
*/
#include "debug.h"
#include <WiFi.h>

bool motor_switch = false;
unsigned long lastSeen = 1;



/*
   SETUP
*/
void setup() {

  // Log
  LOGSETUP();

  // Settings config
  String keys[16] = {"id", "gain"};
  settings_load( keys );

  // Settings SET
  //settings_set("id", 2);
  settings_set("gain", 30);

  // Wifi
  //wifi_static("192.168.0.237");
  wifi_connect("audioscope", "oyomiami");
  wifi_ota( "audioscope-" + String(settings_get("id")) + " v" + String(MP_VERSION, 2) );
  wifi_maxTry(3); // switch off wifi if no network found

  // Audio
  if (!audio_setup()) {
    LOG("No SD detected.. restarting");
    delay(500);
    ESP.restart();
  }
  audio_loop(false);

  //Uart NFC
  nfc_setup();
  while (!nfc_running()) delay(10);
  if (nfc_error()) {
    LOG("No RFID detected.. restarting");
    delay(500);
    ESP.restart();
  }

  // Motor switch
  pinMode(PIN_MOTOR, INPUT);

  // Blink LED (thread)
  led_setup();

}

/*
   LOOP
*/
void loop() {

  wifi_loop();
  audio_run();

  motor_switch = (digitalRead(PIN_MOTOR) == LOW);
  // motor_switch = true;

  // Stop all when motor stop
  if (!motor_switch && lastSeen > 0) {
    LOG("Motor is off: stop all");
    stop_all();
  }

  // Check NFC
  if (nfc_available()) {
    String media = "/"+nfc_get()+".wav";

    // If motor is running && media changed
    if (motor_switch) {
      if (audio_media() != media ) {
        if (audio_play(media)) led_start();
      }

      LOGINL("Ping time: ");
      LOG((millis()-lastSeen));
      lastSeen = millis();
    }

    LOGINL("Ping time: ");
    LOG((millis() - lastSeen));
    lastSeen = millis();

  }

  // Check RFID timeout
  if (motor_switch && audio_running()) {

    if ((millis() - lastSeen) > 2000) {
      LOGINL("No disc seen for a while: stop all: ");
      LOG((millis() - lastSeen));
      stop_all();
    }
    //LOG(counter);
  }

  // Audio did end: stop leds !
  if (led_running() && !audio_running()) led_stop();
}

void stop_all() {
  audio_stop();
  led_stop();
  lastSeen = 0;
}
