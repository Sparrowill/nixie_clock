#include "Omnixie_NTDB.h"

Omnixie_NTDB::Omnixie_NTDB(uint8_t pin_DataIN, 
	uint8_t pin_LatchSTCP, 
	uint8_t pin_ClockSHCP, 
	uint8_t pin_BlankOE, 
	uint8_t pin_HVenable,
	uint8_t pin_colon, 
	byte ntdb_count) :
		_pin_DataIN(pin_DataIN), 
		_pin_LatchSTCP(pin_LatchSTCP), 
		_pin_ClockSHCP(pin_ClockSHCP), 
		_pin_BlankOE(pin_BlankOE), 
		_pin_HV_SHDN(pin_HVenable), 
		_pin_Colon_Ctrl(pin_colon), 
		_ntdb_count(ntdb_count)
{
	_cache_length_bytes = ntdb_count * 5;
	_data = (byte *) malloc(sizeof(byte) * ntdb_count); 

	pinMode(_pin_DataIN, OUTPUT);
	pinMode(_pin_LatchSTCP, OUTPUT);
	pinMode(_pin_ClockSHCP, OUTPUT);
	pinMode(_pin_BlankOE, OUTPUT);
	pinMode(_pin_HV_SHDN, OUTPUT);
	pinMode(_pin_Colon_Ctrl, OUTPUT);

	// this->setBrightness();

	this->clear();

	memset(_data, 0, _cache_length_bytes);// use memset to fill the "_data" array with a single value
	//Sets the first "_cache_length_bytes" bytes of the block of memory pointed by "_data" to the specified value ("0" here)
	//(interpreted as an unsigned char).
}

void Omnixie_NTDB::loadData(byte data, BitOrder bo) const
{
	switch (bo)
	{
	case LSB:
		for (byte i = 0; i < 8; i++)
		{
			digitalWrite(_pin_DataIN, bitRead(data, i));
			digitalWrite(_pin_ClockSHCP, LOW);
			digitalWrite(_pin_ClockSHCP, HIGH);
		}
		break;

	case MSB:
		for (byte i = 7; i >= 0; i--)
		{
			digitalWrite(_pin_DataIN, bitRead(data, i));
			digitalWrite(_pin_ClockSHCP, LOW);
			digitalWrite(_pin_ClockSHCP, HIGH);
		}
		break;

	default:
		break;
	}
}

void Omnixie_NTDB::display()
{
	for (byte i = 0; i < _ntdb_count * 5; i++)
	{
		this->loadData(_data[i], LSB);
	}

	digitalWrite(_pin_LatchSTCP, LOW);
	digitalWrite(_pin_LatchSTCP, HIGH);
}

void Omnixie_NTDB::setText(char charArr[]) {
	int num = 0;
	for (int i = 0; i < 4; i++) {
		if(isDigit(charArr[i])) {
			num += (charArr[i] - '0') * 1000 / (pow(10, i));
		}
	}
	this->setNumber(num, 4);
}

void Omnixie_NTDB::setNumber(unsigned int number, byte DigitDisplayMask)
{
	if (number < 0) number = 0;
	if (number > 9999) number = 9999;
	//use DigitDisplayMask to determine which tube goes dark (not displaying anything)
	//When it's set to 10, the corresponding tube goes dark.
    int digitLeft1 = (DigitDisplayMask & 0b1000) == 0b1000 ? (number / 1000) % 10 : 10;
    int digitLeft2 = (DigitDisplayMask & 0b0100) == 0b0100 ? (number / 100) % 10 : 10;
    int digitLeft3 = (DigitDisplayMask & 0b0010) == 0b0010 ? (number / 10) % 10 : 10;
    int digitLeft4 = (DigitDisplayMask & 0b0001) == 0b0001 ? number % 10 : 10;

    _data[0] = digit[digitLeft4] & 0b11111111;
    _data[1] = ((digit[digitLeft4] & 0b1100000000) >> 8) | ((digit[digitLeft3] & 0b111111) << 2);
    _data[2] = ((digit[digitLeft3] & 0b1111000000) >> 6) | ((digit[digitLeft2] & 0b1111) << 4);
    _data[3] = ((digit[digitLeft2] & 0b1111110000) >> 4) | ((digit[digitLeft1] & 0b11) << 6);
    _data[4] = ((digit[digitLeft1] & 0b1111111100) >> 2);
}

void Omnixie_NTDB::setHVPower(bool hv)
{
	digitalWrite(_pin_HV_SHDN, hv? HIGH : LOW);
}

// void Omnixie_NTDB::setColon(bool colon)
// {
// 	digitalWrite(_pin_Colon_Ctrl, colon);
// }

void Omnixie_NTDB::setColon(byte brightness)
{
	if (digitalPinToTimer(_pin_Colon_Ctrl) == NOT_ON_TIMER)
		{Serial.println("high low");
		digitalWrite(_pin_Colon_Ctrl, brightness ? HIGH : LOW);}
	else
		analogWrite(_pin_Colon_Ctrl, brightness);
}

void Omnixie_NTDB::setBrightness(byte brightness)
{
	if (digitalPinToTimer(_pin_BlankOE) == NOT_ON_TIMER)
		digitalWrite(_pin_BlankOE, brightness ? LOW : HIGH);
	else
		analogWrite(_pin_BlankOE, 0xff - brightness);
}

void Omnixie_NTDB::putWord(byte index, word value)
{
	index %= _ntdb_count;
	_data[index] = value;
}

void Omnixie_NTDB::clear(word value)
{
	for (byte i = 0; i < _ntdb_count; i++)
		this->putWord(i, value);
}

Omnixie_NTDB::~Omnixie_NTDB()
{
	free(_data);
}

