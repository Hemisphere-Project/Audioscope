/*
   SETTINGS
*/
#define MP_VERSION  0.4

#define PIN_MOTOR 15

/*
   INCLUDES
*/
#include "debug.h"
#include <WiFi.h>

bool motor_switch = false;
int counter = 0;



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
  settings_set("id", 1);
  settings_set("gain", 80);

  // Wifi
  wifi_static("192.168.0.237");
  wifi_connect("101WIFI", "101internet");
  wifi_ota( "audioscope-" + String(settings_get("id")) + " v" + String(MP_VERSION, 2) );
  wifi_onConnect(osc_setup);
  //wifi_wait(5000, true);

  // Audio
  if (!audio_setup()) {
    LOG("No SD detected.. restarting");
    delay(500);
    ESP.restart();
  }
  audio_loop(true);

  //Uart NFC
  nfc_setup();
  while(!nfc_running()) delay(10);
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

  osc_loop();

  audio_run();

  motor_switch = (digitalRead(PIN_MOTOR) == LOW);

  // Stop all when motor stop
  if (!motor_switch) stop_all();

  // Check NFC
  if (nfc_available()) {
    String media = nfc_get()+".mp3";

    // If motor is running
    if (motor_switch) {
      
      if (audio_media() != "/"+media ) {
        if (audio_play(media)) led_start();
      }
      counter = 0;
    }
  }

  if (motor_switch && (audio_running() || led_running())) {
    counter += 1;
    if (counter > 6000) stop_all();
    LOG(counter);
  }
  
}

void stop_all() {
  if (audio_running()) audio_stop();
  if (led_running()) led_stop();
  counter = 0;
}







