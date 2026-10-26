#include "time_conversion.h"
// Called if displaying a timer
int get_nixie_numbers(int remaining) {
  // Take in s remaining, return a int showing what numbers need to display on clock
  int nixie_numbers = 0;
  int pairOne = 0;
  int pairTwo = 0;
  unsigned long hours = 0;
  unsigned long minutes = 0;
  unsigned long seconds = 0;
  // Process remaining
  if (remaining > 3600 /*1 hour*/) {  // Display hours:minutes
    // Display hours and minutes
    hours = remaining / 3600;
    hours %= 24;
    pairOne = hours;
    minutes = remaining / 60;
    minutes %= 60;
    pairTwo = minutes;
  } else {
    // Display minutes:seconds
    minutes = remaining / 60;
    minutes %= 60;
    pairOne = minutes;
    seconds = remaining;
    seconds %= 60;
    pairTwo = seconds;
  }
  nixie_numbers += (100 * pairOne);
  nixie_numbers += pairTwo;
  Serial.println(nixie_numbers);
  return nixie_numbers;
}