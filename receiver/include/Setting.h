#ifndef _RX_SETTING_H
#define _RX_SETTING_H

class SETTING {
public:
	DATA_EEPROM e;
  
  void load(RECEIVER * RP);
	void save(void);
	void setValue( uint8_t index, uint8_t value);
	uint8_t getValue(uint8_t index);
  void readAddress(void);

private:
  RECEIVER * RP;
  
  bool checkValue(uint8_t value, uint8_t index);
  void writeAddress(void);
};

#endif