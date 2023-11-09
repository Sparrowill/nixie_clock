#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <WiFi.h>
#include <PubSubClient.h>

// WiFi
const char *ssid = "27WiFi";        // Enter your WiFi name
const char *password = "Connect!";  // Enter WiFi password
WiFiClient espClient;

// MQTT Broker
const char *mqtt_broker = "192.168.1.41";
const char *command_topic = "nixie_clock/set";
const char *online_topic = "nixie_clock/available";
const char *state_topic = "nixie_clock/remaining";
const char *time_topic = "nixie_clock/timestamp";
const char *mqtt_username = "mqtt";
const char *mqtt_password = "MQTTpassword1!";
const int mqtt_port = 1883;
const String client_id = "nixie_clock";
PubSubClient client(espClient);

enum STATE {
  CLOCK,
  TIMER_ACTIVE,
  TIMER_END
};

STATE state = CLOCK;


#endif //CONSTANTS_H