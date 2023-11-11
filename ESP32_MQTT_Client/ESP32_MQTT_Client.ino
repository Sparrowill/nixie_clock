#include "constants.h"
#include "helpers.h"
#include "Omnixie_NTDB.h"

// Timing variables
int currentHour = 0;
int currentMin = 0;
int msRemaining = 0;
unsigned long timestamp = 0;
bool got_new_clock = false;

#define NTDB_count 1
// define how many NTDB boards in use

Omnixie_NTDB nixieClock(11, 8, 12, 10, 6, 5, NTDB_count);
// pin_DataIN, pin_STCP(latch), pin_SHCP(clock), pin_Blank(Output Enable; PWM pin preferred),
// HVEnable pin, Colon pin, number of Nixie Tube Driver Boards

void wifi_connect(const char *ssid, const char *password) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

void nixie_setup() {
  //turn on the high voltage provided by NCH6300HV
  nixieClock.setHVPower(true);

  // Brightness control, range 0x00(off) to 0xff(brightest).
  nixieClock.setBrightness(0xff);
  //turn on the tube display
  nixieClock.display();
}
void CathodePoisoningPrevention(unsigned int num, int msDelay) {
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

void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);
  // connecting to a WiFi network
  wifi_connect(ssid, password);
  // Nixie Setup
  nixie_setup();
  CathodePoisoningPrevention(3, 1000);

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


void loop() {
  unsigned long msElapsed = 0;
  unsigned long last_timer_update = 0;
  unsigned long last_clock_update = 0;
  String msRemaining_str = "";
  int nn = 0;
  bool update_timer = false;
  while (1) {
    client.loop();
    switch (state) {

      case CLOCK:
        // Update the clock when the time changes
        if (got_new_clock) {
          got_new_clock = false;
          nn = get_nixie_numbers(currentHour, currentMin);
          nixieClock.setNumber(nn, 0b1111);
          //Light up the tubes
          nixieClock.display();
        }
        // If the time is midnight, do cathode poisoning prevention
        if (nn == 2400){
          CathodePoisoningPrevention(3, 1000);
        }
        break;

      case TIMER_ACTIVE:
        // Calculate milliseconds remaining
        msElapsed = millis() - timestamp;
        // Set last calculated time
        timestamp = millis();
        // Update timer
        msRemaining -= msElapsed;
        // If there is less than a minute left, up the refresh rate to accout for milliseconds
        if (msRemaining < 60000) {
          nn = get_nixie_numbers(msRemaining);
          update_timer = true;
        }
        // Otherwise only refresh if a second has passed since the last refresh
        if (millis() - last_timer_update > 1000) {
          last_timer_update = millis();
          update_timer = true;
          // Publish last update back to Home Assistant
          msRemaining_str = "{\"remaining\": " + String(msRemaining / 1000) + "}";
          client.publish(state_topic, msRemaining_str.c_str());
        }
        if (update_timer) {
          update_timer = false;
          nn = get_nixie_numbers(msRemaining);
          // Display time remaining
          nixieClock.setNumber(nn, 0b1111);
          nixieClock.display();
        }
        if (msRemaining <= 0) {
          state = TIMER_END;
        }
        break;

      case TIMER_END:
        // Flash the display with 4 0's
        Serial.println("Timer done");
        for (int i = 0; i < 6; i++) {
          nixieClock.setNumber(0000, 0b1111);
          nixieClock.display();
          delay(500);
          nixieClock.setBrightness(0x00);
          nixieClock.display();
          delay(500);
          nixieClock.setBrightness(0xff);
        }
        state = CLOCK;
        break;

      default:
        state = CLOCK;
        break;
    }
  }
}
