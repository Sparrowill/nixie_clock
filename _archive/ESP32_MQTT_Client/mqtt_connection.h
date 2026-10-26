#ifndef MQTT_CONNECTION_H
#define MQTT_CONNECTION_H

#include <Arduino.h>
#include <PubSubClient.h>

enum STATE {
  CLOCK,
  TIMER_ACTIVE,
  TIMER_END
};

// MQTT items used in the main .ino
extern STATE state;

// Timing variables
extern int currentHour;
extern int currentMin;
extern int msRemaining;
extern unsigned long timestamp;
extern bool got_new_clock;


void callback(char *rx_topic, byte *payload, unsigned int length);
void mqtt_connect();
void update_state(String payload);
void update_mqtt();

#endif  //MQTT_CONNECTION_H