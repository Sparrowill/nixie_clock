#ifndef Omnixie_NTDB_H_
#define Omnixie_NTDB_H_

#include "Arduino.h"

enum Colon {
  None,
  Upper,
  Lower,
  Both
};

enum BitOrder {
  LSB,
  MSB
};

class Omnixie_NTDB {

public:
  Omnixie_NTDB(uint8_t pin_DataIN,
               uint8_t pin_LatchSTCP,
               uint8_t pin_ClockSHCP,
               uint8_t pin_BlankOE,
               uint8_t pin_HVenable,
               uint8_t pin_colon,
               byte ntdb_count = 1  // qty of NTDB boards
  );

  virtual ~Omnixie_NTDB();

  void clear(word value = 0x7000);  //16-bit word

  void putWord(byte index, word value = 0x7000);

  void setBrightness(byte brightness = 0x40);
  void display();

  void setHVPower(boolean hv = false);

  void setNumber(unsigned int num, byte DigitDisplayMask);
  void setText(char charArr[]);

  // void setColon(boolean colonOn = false);
  void setColon(byte brightness);

private:

  // word *_buff; //store an unsigned number of at least 16 bits (from 0 to 65535).
  // char * _cache; //8 bit
  byte* _data;  // 8 bit array

  const uint8_t _pin_DataIN;      // Data In
  const uint8_t _pin_LatchSTCP;   // Latch Pin (STCP)
  const uint8_t _pin_ClockSHCP;   // Clock Pin (SHCP)
  const uint8_t _pin_BlankOE;     // Blank Pin (OE). Although any I/O can be used, a PWM pin is preferred to manipulate brightness.
  const uint8_t _pin_HV_SHDN;     // HV SHDN Pin
  const uint8_t _pin_Colon_Ctrl;  // Colon Ctrl Pin
  const byte _ntdb_count;

  byte _cache_length_bytes;

  void loadData(byte data, BitOrder order) const;

  // from left 9 to right 0; 7618325409; check the board schematic to find out why
  const int digit[11] = {
    0b0000000010,  //0
    0b0010000000,  //1
    0b0000010000,  //2
    0b0000100000,  //3
    0b0000000100,  //4
    0b0000001000,  //5
    0b0100000000,  //6
    0b1000000000,  //7
    0b0001000000,  //8
    0b0000000001,  //9
    0b0000000000   //display off
  };
};

#endif /* Omnixie_NTDB_H_ */
