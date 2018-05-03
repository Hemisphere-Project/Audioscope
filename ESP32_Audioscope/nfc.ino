#include <PN532_HSU.h>
#include <PN532.h>

#include "HardwareSerial.h"

uint8_t nfc_uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t nfc_uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

bool nfc_run = false;
bool nfc_err = false;
bool nfc_didread = false;

void nfc_setup() {
  xTaskCreatePinnedToCore(
    nfc_task,   /* Function to implement the task */
    "nfcy", /* Name of the task */
    20000,      /* Stack size in words */
    NULL,       /* Task input parameter */
    1,          /* Priority of the task */
    NULL,       /* Task handle. */
    0);  /* Core where the task should run */
}

void nfc_task(void *pvParameter)
{
  HardwareSerial Serial2(2);

  PN532_HSU pn532hsu(Serial2);
  PN532 nfc(pn532hsu);

  //UART NFC
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) nfc_err = true;
  
  // configure board to read RFID tags
  nfc.SAMConfig();

  nfc_run = true;
  LOG("NFC ready");

  while (!nfc_err) {
    delay(10);
    while (nfc_didread) delay(10);
    bool success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &nfc_uid[0], &nfc_uidLength, 200);
    if (success) nfc_didread = true;
    if (success) LOG("NFC ping");
  }
  vTaskDelete( NULL );
}

bool nfc_available() {
  return nfc_didread;
}

String nfc_get() {
  String uid = "";
  for (uint8_t i = 0; i < nfc_uidLength; i++) uid += String(nfc_uid[i], HEX);
  nfc_didread = false;
  return uid;
}

bool nfc_error() {
  return nfc_err;
}

bool nfc_running() {
  return nfc_run;
}

