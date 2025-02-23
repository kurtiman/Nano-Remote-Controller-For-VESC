#include "Receiver.h"

#include <EEPROM.h>

void SETTING::load(RECEIVER * RP){
	this->RP = RP;

	e.BatteryType	= EEPROM.read(0x00);
	e.BatterySerie	= EEPROM.read(0x01);
	e.motorPoles	= EEPROM.read(0x02);
	e.motorPulley	= EEPROM.read(0x03);
	e.wheelPulley	= EEPROM.read(0x04);
	e.wheelSize		= EEPROM.read(0x05);
	e.Stepper		= EEPROM.read(0x06);
	e.SpeedLimit	= EEPROM.read(0x07);
	readAddress();

	for(uint8_t i=0; i<SETTING_NUM; i++){
		if(SETTING_RULE[i][0]>0){
			if(!checkValue(getValue(i), i)){
				setValue(i, SETTING_RULE[i][0]);
			}
		}
	}
}

void SETTING::save(void){
	EEPROM.update(0x00, e.BatteryType & 0xFF);
	EEPROM.update(0x01, e.BatterySerie & 0xFF);
	EEPROM.update(0x02, e.motorPoles & 0xFF);
	EEPROM.update(0x03, e.motorPulley & 0xFF);
	EEPROM.update(0x04, e.wheelPulley & 0xFF);
	EEPROM.update(0x05, e.wheelSize & 0xFF);
	EEPROM.update(0x06, e.Stepper & 0xFF);
	EEPROM.update(0x07, e.SpeedLimit & 0xFF);
	writeAddress();
}

uint8_t SETTING::getValue(uint8_t index){
	switch(index){
		case 0: return e.BatteryType; break;
		case 1: return e.BatterySerie; break;
		case 2: return e.motorPoles; break;
		case 3: return e.motorPulley; break;
		case 4: return e.wheelPulley; break;
		case 5: return e.wheelSize; break;
		case 6: return e.Stepper; break;
		case 7: return e.SpeedLimit; break;
	}
	return 0;
}

void SETTING::setValue(uint8_t index, uint8_t value){
	value = constrain(value, SETTING_RULE[index][1], SETTING_RULE[index][2]);
	switch(index){
		case 0: e.BatteryType	= value; break;
		case 1: e.BatterySerie	= value; break;
		case 2: e.motorPoles	= value; break;
		case 3: e.motorPulley	= value; break;
		case 4: e.wheelPulley	= value; break;
		case 5: e.wheelSize		= value; break;
		case 6: e.Stepper		= value; break;
		case 7: e.SpeedLimit	= value; break;
	}
}

bool SETTING::checkValue(uint8_t value, uint8_t index){
	return (value < SETTING_RULE[index][1] || value > SETTING_RULE[index][2]) ? false : true;
}

void SETTING::readAddress(void){
	uint8_t block = SETTING_NUM-1;
	e.addr=0x00;
	uint8_t x = 0x00;
	uint8_t shift = 0x28;
	do{
		e.addr+=(uint64_t)EEPROM.read(block+x)<<(shift-=0x08);
		x+=0x01;
	}while(x < 0x05);
	if(!RP->NRF.checkAddress(e.addr)){
		e.addr = DEFAULT_ADDR;
	}else{
		RP->NRF.Tx.addr = e.addr;
	}
}

void SETTING::writeAddress(void){
	uint8_t block = SETTING_NUM-1;
	EEPROM.update(block, (e.addr>>0x20) & 0xFF);
	EEPROM.update(block+0x01, (e.addr>>0x18) & 0xFF);
	EEPROM.update(block+0x02, (e.addr>>0x10) & 0xFF);
	EEPROM.update(block+0x03, (e.addr>>0x08) & 0xFF);
	EEPROM.update(block+0x04, e.addr);
}