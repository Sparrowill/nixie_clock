/*
For programming OTA - use https://dronebotworkshop.com/esp32-ota/


*/

#include "Omnixie_NTDB.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "time.h"
#include <PubSubClient.h>
#include "time_conversion.h"



const char *mqtt_broker = "192.168.1.41";
const char *online_topic = "nixie_clock/available";
const char *state_topic = "home/kitchen/timer";
const char *mqtt_username = "mqtt";
const char *mqtt_password = "password";
const int mqtt_port = 1883;
const String client_id = "nixie_clock-esp32";
WiFiClient espClient;
PubSubClient client(espClient);

// Timing variables
int remaining = 0;

const char *ssid = "27WiFi";
const char *password = "Connect!";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;


enum STATE {
  CLOCK,
  TIMER_ACTIVE,
  WAIT,
  TIMER_END
};
STATE state;

bool timer_live = false;
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


  ArduinoOTA.setHostname("Test-ESP32S3");
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
  mqtt_connect();
}

void loop() {
  unsigned long enter_time = millis();
  CathodePoisoningPrevention(3, 1000);

  struct tm timeinfo;
  state = CLOCK;

  while (true) {

    if (state == WAIT) {
      //Wait for a minute
      if (((millis() - enter_time) > 60000) && !timer_live) {
        state = CLOCK;
      }
      if (!client.connected()) {
        Serial.println("MQTT disconnected - reconnecting...");
        mqtt_connect();
      }
      client.loop();
      ArduinoOTA.handle();
    }
    if (state == CLOCK) {
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
      enter_time = millis();
      state = WAIT;
    }
    if (state == TIMER_ACTIVE) {
      timer_live = true;
      int num = get_nixie_numbers(remaining);
      nixieClock.setNumber(num, 0b1111);
      nixieClock.display();

      state = WAIT;
    }
    if (state == TIMER_END) {
      timer_live = false;
      Serial.println("Timer Ends");
      for (int i = 0; i < 6; i++) {
        nixieClock.setBrightness(0xff);
        nixieClock.setNumber(0, 0b1111);
        //Light up the tubes
        nixieClock.display();
        delay(500);
        nixieClock.setBrightness(0x00);
        nixieClock.display();
        delay(500);
        nixieClock.setBrightness(0xff);
      }
      state = CLOCK;
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



// Called whenever a new message is received on a topic we are subscribed to
void callback(char *rx_topic, byte *payload, unsigned int length) {

  String str_payload = "";
  for (int i = 0; i < length; i++) {
    str_payload += (char)payload[i];
  }
  remaining = str_payload.toInt();

  if (remaining > 1) {
    state = TIMER_ACTIVE;
  } else {
    state = TIMER_END;
    return;
  }
  // Remind Home Assistant we're still alive when time updates (once a minute)
  client.publish(online_topic, "online");
}

void mqtt_connect() {
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Client connecting: ");
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
  // Tell Home Assistant that we are online
  client.publish(online_topic, "online");
  // Start listening to new timers coming in
  client.subscribe(state_topic);
}
