#ifndef _TX_SETTING_H
#define _TX_SETTING_H

class SETTING {
public:
  DATA_EEPROM e;

  void load(CONTROLLER * CP);
  void save();
	void setValue(byte index, short value);
	short getValue(byte index);

private:
  CONTROLLER * CP;

  uint8_t Optional;

  void writeAddressArray(void);
  void readAddressArray(void);
  bool checkValue(uint8_t value, uint8_t index);
};

#endif