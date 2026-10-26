#ifndef NIXE_CONTROL
#define NIXE_CONTROL

#include <Arduino.h>

// define how many NTDB boards in use
#define NTDB_count 1

void nixie_setup();
void cathode_poisoning_prevention(unsigned int num, int msDelay);
void show_nixie(int nn, byte tubes);
void nixie_off();
int get_nixie_numbers(int msRemaining);
int get_nixie_numbers(int currentHour, int currentMin);

#endif //NIXIE_CONTROL