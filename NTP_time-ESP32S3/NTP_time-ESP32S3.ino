/*
For programming OTA - use https://dronebotworkshop.com/esp32-ota/


  Nixie Tube Driver Board (NTDB) 4-Nixie Tube Display Minimal Example

  This example demonstrates a minimal display using 4 nixie tubes.
  No colon bulbs. No buttons/controls. No clock.
  See aother example we provided for a complete clock.

  In order to make a complete clock, you will need:
  1) This KIT: https://nixie.ai/ntdb4
  2) 4x nixie tubes
  3) 1x NCH6300HV: https://nixie.ai/nch6300hv
  4) 1x Arduino Uno board
  5) 1x Real Time Clock module (DS1307, DS3231, etc)
  6) Some jumper wires


  Circuit:
  Connect the NTDB to Arduino Uno:
  --------------------------------
    NTDB        Arduino Pins
  --------------------------------
    GND         GND
    DC5V
    DATA        11
    OE          10
    STCP        8
    SHCP        12
    COLON       5 (Not In Use)
    ON/OFF      6 (HVEnable)
  --------------------------------
  Connect the 12V DC power to the NTDB board 


  Nixie Tube Driver Board (NTDB) Arduino Library released at
  https://github.com/omnixie-electronics/Nixie-Tube-Driver-Board-Arduino-Library
  with short link: https://nixie.ai/ntdb4github

  First released in public domain on Thanksgiving day, 11/26/2020
  Library and example code created
  by Aiden Fang
  www.Omnixie.com

  NTDB board designed by Zeyuan, Yan
  www.Omnixie.cn

*/

#include "Omnixie_NTDB.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "time.h"

const char* ssid = "27WiFi";
const char* password = "Connect!";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

#define NTDB_count 1
// define how many NTDB boards in use

Omnixie_NTDB nixieClock(D1, D2, D3, D4, D5, D6, NTDB_count);
// pin_DataIN, pin_STCP(latch), pin_SHCP(clock), pin_Blank(Output Enable; PWM pin preferred),
// HVEnable pin, Colon pin, number of Nixie Tube Driver Boards
// PWM Pins on Arduino Uno: 3, 5, 6, 9, 10, 11; PWM FREQUENCY 490 Hz (pins 5 and 6: 980 Hz)

void setup() {
  //turn on the high voltage provided by NCH6300HV
  nixieClock.setHVPower(true);

  // Brightness control, range 0x00(off) to 0xff(brightest).
  nixieClock.setBrightness(0xff);
  //turn on the tube display
  nixieClock.display();

  Serial.begin(115200);

  //Connect to Wi-Fi
  WiFi.setSleep(false);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    nixieClock.setNumber(404, 0b1110);
    delay(500);
  }
  Serial.println("Got Wifi");
  // Init time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // OTA programming Setup
  // --- ArduinoOTA Setup ---
  ArduinoOTA.setHostname("Nixie-Clock-XIAO-ESP32S3");

  ArduinoOTA.onStart([]() {
    nixieClock.setNumber(1111, 0b1111);
  });

  ArduinoOTA.onEnd([]() {
    nixieClock.setNumber(0, 0b1111);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
  });
  ArduinoOTA.begin();
}

void loop() {
  CathodePoisoningPrevention(3, 1000);
  struct tm timeinfo;

  while (true) {

    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }
    int currentTime = (100 * timeinfo.tm_hour) + timeinfo.tm_min;
    Serial.println(currentTime);
    //Midnight Check
    if (currentTime > 2358) {
      return;  //do cathode poisoning again.
    }

    nixieClock.setNumber(currentTime, 0b1111);
    nixieClock.display();
    unsigned long enter_time = millis();
    //Wait for a minute, adjusting for drift
    while ((millis() - enter_time) < (60000 - (timeinfo.tm_sec * 1000))) {

      ArduinoOTA.handle();
    }
  }
}

void CathodePoisoningPrevention(unsigned int num, int msDelay) {
  if (num < 1) exit;
  for (byte n = 0; n < num; n++) {
    Serial.println("Running Cathode Poisoning Prevention ... ");
    for (byte i = 0; i < 10; i++) {
      nixieClock.setNumber(i * 1111, 0b1111);
      nixieClock.display();
      delay(msDelay);
    }
  }
  delay(1000);
}