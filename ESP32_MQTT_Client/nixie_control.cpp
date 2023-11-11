#include "nixie_control.h"
#include <Omnixie_NTDB.h>

Omnixie_NTDB nixieClock(11, 8, 12, 10, 6, 5, NTDB_count);
// pin_DataIN, pin_STCP(latch), pin_SHCP(clock), pin_Blank(Output Enable; PWM pin preferred),
// HVEnable pin, Colon pin, number of Nixie Tube Driver Boards

void nixie_setup() {
  //turn on the high voltage provided by NCH6300HV
  nixieClock.setHVPower(true);
  // Brightness control, range 0x00(off) to 0xff(brightest).
  nixieClock.setBrightness(0xff);
  //turn on the tube display
  nixieClock.display();
}

void cathode_poisoning_prevention(unsigned int num, int msDelay) {
  for (byte n = 0; n < num; n++) {
    Serial.println("Running Cathode Poisoning Prevention ... ");
    for (byte i = 0; i < 10; i++) {
      show_nixie(i * 1111, 0b1111);
      delay(msDelay);
    }
  }
  delay(1000);
}

void show_nixie(int nn, byte tubes) {
  nixieClock.setBrightness(0xff);
  nixieClock.setNumber(nn, tubes);
  //Light up the tubes
  nixieClock.display();
}

void nixie_off() {
  nixieClock.setBrightness(0x00);
  nixieClock.display();
}

// Called if displaying a timer
int get_nixie_numbers(int msRemaining) {
  // Take in ms remaining, return a int showing what numbers need to display on clock
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