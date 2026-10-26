#include "mqtt_connection.h"
#include "wifi_connection.h"
#include "nixie_control.h"

void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);
  // connecting to a WiFi network
  wifi_connect();
  // Nixie Setup
  nixie_setup();
  cathode_poisoning_prevention(3, 1000);
  mqtt_connect();

  // Set display to CLOCK
  state = CLOCK;
}

void loop() {
  unsigned long msElapsed = 0;
  unsigned long last_timer_update = 0;
  unsigned long last_clock_update = 0;
  int nn = 0;
  bool update_timer = false;
  while (1) {
    update_mqtt();
    switch (state) {
      case CLOCK:
        // Update the clock when the time changes
        if (got_new_clock) {
          got_new_clock = false;
          nn = get_nixie_numbers(currentHour, currentMin);
          show_nixie(nn, 0b1111);
        }
        // If the time is midnight, do cathode poisoning prevention
        if (nn == 2400) {
          cathode_poisoning_prevention(3, 1000);
        }
        break;

      case TIMER_ACTIVE:
        msElapsed = millis() - timestamp;  // Calculate milliseconds remaining
        timestamp = millis();              // Set last calculated time
        msRemaining -= msElapsed;          // Update timer
        // If there is less than a minute left, up the refresh rate to account for milliseconds
        if (msRemaining < 60000) {
          update_timer = true;
        }
        // Otherwise only refresh if a second has passed since the last refresh
        if (millis() - last_timer_update > 1000) {
          last_timer_update = millis();
          update_timer = true;
          update_state(String(msRemaining / 1000));  // Publish last update back to Home Assistant
        }
        if (update_timer) {
          update_timer = false;
          nn = get_nixie_numbers(msRemaining);
          show_nixie(nn, 0b1111);  // Display time remaining
        }
        if (msRemaining <= 0) {
          state = TIMER_END;
        }
        break;

      case TIMER_END:
        // Flash the display with 4 0's
        for (int i = 0; i < 6; i++) {
          show_nixie(0, 0b1111);
          delay(500);
          nixie_off();
          delay(500);
        }
        state = CLOCK;
        break;

      default:
        state = CLOCK;
        break;
    }
  }
}
