/*
   SETTINGS
*/
#define MP_VERSION  0.50   // remove OSC, disable wifi after 15s
#define MP_VERSION  0.51   // timeout RFID in ms instead of counter
#define MP_VERSION  0.52   // pin led task to CPU0
#define MP_VERSION  0.53   // pin led task to ISR

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
  settings_set("id", 2);
  settings_set("gain", 80);

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
    String media = nfc_get()+".mp3";

    // If motor is running
    if (motor_switch) {      
      if (audio_media() != "/"+media ) {
        if (audio_play(media)) led_start();
      }
      
      LOGINL("Ping time: ");
      LOG((millis()-lastSeen));
      lastSeen = millis();
    }
  }

  // Check RFID timeout
  if (motor_switch && (audio_running() || led_running())) {
    
    if ((millis() - lastSeen) > 2000) {
      LOGINL("No disc seen for a while: stop all: ");
      LOG((millis() - lastSeen));
      stop_all();
    }
    //LOG(counter);
  }
  
}

void stop_all() {
  if (audio_running()) audio_stop();
  if (led_running()) led_stop();
  lastSeen = 0;
}







