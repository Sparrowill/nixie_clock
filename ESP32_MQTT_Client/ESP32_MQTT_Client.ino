#include <WiFi.h>
#include <PubSubClient.h>

// WiFi
const char *ssid = "27WiFi";        // Enter your WiFi name
const char *password = "Connect!";  // Enter WiFi password

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

// Timing variables
int currentHour = 0;
int currentMin = 0;
int msRemaining = 0;
unsigned long timestamp = 0;
unsigned long msElapsed = 0;
unsigned long last_update = 0;


WiFiClient espClient;
PubSubClient client(espClient);

enum STATE {
  CLOCK,
  TIMER_ACTIVE,
  TIMER_END
};

STATE state = CLOCK;

void wifi_connect(const char *ssid, const char *password) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

int get_nixie_numbers(bool clock = false) {
  // Take in ms remaining, return a byte showing what numbers need to display on clock
  int nixie_numbers = 0;
  int pairOne = 0;
  int pairTwo = 0;
  unsigned long hours = 0;
  unsigned long minutes = 0;
  unsigned long seconds = 0;
  unsigned long milliseconds = 0;
  if (clock) {
    // Process hrs and minutes
    pairOne = currentHour;
    pairTwo = currentMin;
  } else {
    // Process msRemaining
    if (msRemaining > 3600000 /*1 hour*/) {  // Display hours:minutes
      // Display hours and minutes
      hours = msRemaining / 3600000;
      hours %= 24;
      pairOne = hours;
      minutes = msRemaining / 60000;
      minutes %= 60;
      pairTwo = minutes;
    } else if (msRemaining > 60000 /*1 minute*/) {  // Display minutes:seconds
      // Display minutes and seconds
      minutes = msRemaining / 60000;
      minutes %= 60;
      pairOne = minutes;
      seconds = msRemaining / 1000;
      seconds %= 60;
      pairTwo = seconds;
    } else if (msRemaining > 1000 /*1 Second*/) {  // Display seconds:milliseconds
      seconds = msRemaining / 1000;
      seconds %= 60;
      pairOne = seconds;
      milliseconds = msRemaining % 1000;
      pairTwo = milliseconds / 10;
    }
  }
  nixie_numbers += (100 * pairOne);
  nixie_numbers += pairTwo;
  Serial.print("Nixie :");
  Serial.println(nixie_numbers);
  return nixie_numbers;
}

void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);
  // connecting to a WiFi network
  wifi_connect(ssid, password);
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
    get_nixie_numbers(true);
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
  String msRemaining_str = "";
  while (1) {

    client.loop();
    switch (state) {
      case CLOCK:
        // TODO Display current hour and minute values
        break;
      case TIMER_ACTIVE:
        // Calculate milliseconds remaining
        msElapsed = millis() - timestamp;
        // Set last calculated time
        timestamp = millis();
        msRemaining -= msElapsed;
        // Convert seconds remaining to HH:MM or MM:SS or SS:mmX
        // If there is less than a minute left, up the refresh rate
        if (msRemaining < 60000) {
          get_nixie_numbers();
          // Otherwise only refresh if a second has passed since the last refresh
        } else if (millis() - last_update > 1000) {
          get_nixie_numbers();
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
