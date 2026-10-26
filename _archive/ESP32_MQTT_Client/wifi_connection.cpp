#include "wifi_connection.h"
#include <WiFi.h>

const char *ssid = "27WiFi";        // Enter your WiFi name
const char *password = "Connect!";  // Enter WiFi password

void wifi_connect() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}