#include "constants.h"
#include "helpers.h"

// Timing variables
int currentHour = 0;
int currentMin = 0;
int msRemaining = 0;
unsigned long timestamp = 0;


void wifi_connect(const char *ssid, const char *password) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

void mqtt_connect() {
}

void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);
  // connecting to a WiFi network
  wifi_connect(ssid, password);
  //connecting to a mqtt broker
  // mqtt_connect();
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
  // Set display to CLOCK
  state = CLOCK;
}

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
    get_nixie_numbers(currentHour, currentMin);
    // Remind Home Assistant we're still alive when time updates (once a minute)
    client.publish(online_topic, "online");
  } else {  // Payload is a new timer
    // New timer started
    state = TIMER_ACTIVE;
    // Mark when timer started
    timestamp = millis();
    // Set number of ms to run timer for
    msRemaining = str_payload.toInt() * 1000;
  }
}


void loop() {
  unsigned long msElapsed = 0;
  unsigned long last_update = 0;
  String msRemaining_str = "";
  while (1) {
    client.loop();
    switch (state) {
      case CLOCK:
        // TODO Display current hour and minute values when clock changes

        break;
      case TIMER_ACTIVE:
        // Calculate milliseconds remaining
        msElapsed = millis() - timestamp;
        // Set last calculated time
        timestamp = millis();
        msRemaining -= msElapsed;
        // Convert seconds remaining to HH:MM or MM:SS or SS:mm
        // If there is less than a minute left, up the refresh rate
        if (msRemaining < 60000) {
          get_nixie_numbers(msRemaining);
          // Otherwise only refresh if a second has passed since the last refresh
        }
        if (millis() - last_update > 1000) {
          get_nixie_numbers(msRemaining);
          last_update = millis();
          // Publish last update back to Home Assistant
          msRemaining_str = "{\"remaining\": " + String(msRemaining / 1000) + "}";
          client.publish(state_topic, msRemaining_str.c_str());
        }
        // TODO Display time remaining

        if (msRemaining <= 0) {
          state = TIMER_END;
        }
        break;
      case TIMER_END:
        // TODO Flash the display with 4 0's
        Serial.println("Timer done");
        state = CLOCK;
        break;
      default:
        state = CLOCK;
        break;
    }
  }
}
