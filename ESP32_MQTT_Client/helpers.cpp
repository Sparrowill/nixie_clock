#include "helpers.h"

// Called if displaying a timer
int get_nixie_numbers(int msRemaining) {
  // Take in ms remaining, return a byte showing what numbers need to display on clock
  int nixie_numbers = 0;
  int pairOne = 0;
  int pairTwo = 0;
  unsigned long hours = 0;
  unsigned long minutes = 0;
  unsigned long seconds = 0;
  unsigned long milliseconds = 0;
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
  nixie_numbers += (100 * pairOne);
  nixie_numbers += pairTwo;
  Serial.print("Nixie :");
  Serial.println(nixie_numbers);
  return nixie_numbers;
}

// Called if displaying a clock
int get_nixie_numbers(int currentHour, int currentMin) {
  // Take in current time, return as 4 dgit int
  int nixie_numbers = 0;
  nixie_numbers += (100 * currentHour);
  nixie_numbers += currentMin;
  Serial.print("Nixie :");
  Serial.println(nixie_numbers);
  return nixie_numbers;
}