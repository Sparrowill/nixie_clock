#include "mqtt_connection.h"
#include <WiFi.h>


const char *mqtt_broker = "192.168.1.41";
const char *command_topic = "nixie_clock/set";
const char *online_topic = "nixie_clock/available";
const char *time_topic = "nixie_clock/timestamp";
const char *new_timer_topic = "nixie_clock/new_timer";
const char *mqtt_username = "mqtt";
const char *mqtt_password = "MQTTpassword1!";
const int mqtt_port = 1883;
const String client_id = "nixie_clock";
const char *state_topic = "nixie_clock/remaining";
WiFiClient espClient;
PubSubClient client(espClient);

// Timing variables
extern int currentHour = 0;
extern int currentMin = 0;
extern int msRemaining = 0;
extern unsigned long timestamp = 0;
extern bool got_new_clock = false;

extern STATE state = CLOCK;

// Called whenever a new message is received on a topic we are subscribed to
void callback(char *rx_topic, byte *payload, unsigned int length) {
  String str_payload = "";
  for (int i = 0; i < length; i++) {
    str_payload += (char)payload[i];
  }
  // If payload is a clock update
  if (str_payload.indexOf(":") > 0) {
    // Update clock values
    currentHour = str_payload.substring(0, 2).toInt();
    currentMin = str_payload.substring(3, 5).toInt();
    got_new_clock = true;
    // Remind Home Assistant we're still alive when time updates (once a minute)
    client.publish(online_topic, "online");
  } else {  // Payload is a new timer
    // Mark when timer started
    timestamp = millis();
    // Set number of ms to run timer for
    msRemaining = str_payload.toInt() * 1000;
    // Tell the rest of Home Assistant that we have started a timer
    String str = String(msRemaining / 1000);
    client.publish(new_timer_topic, str.c_str());
    // Trigge the change to Timer on nixie display
    state = TIMER_ACTIVE;
  }
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
  client.subscribe(command_topic);
  client.subscribe(time_topic);
}

void update_state(String payload) {
  String payload_json = "{\"remaining\": " + payload + "}";
  client.publish(state_topic, payload_json.c_str());
}
void update_mqtt(){
  // Single line function placed in here for reduced scope of client
      client.loop();
}