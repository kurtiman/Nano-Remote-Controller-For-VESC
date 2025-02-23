#include "Controller.h"
#include "Setting.h"

#include <EEPROM.h>

void SETTING::load(CONTROLLER * CP){
	this->CP = CP;

	e.Profile 		= EEPROM.read(0x00);
	e.Mode 			= EEPROM.read(0x01);
	e.HallCenter	= (EEPROM.read(0x02) << 0x08) + EEPROM.read(0x03);
	e.HallMin 		= EEPROM.read(0x04);
	e.HallMax 		= (EEPROM.read(0x05) << 0x08) + EEPROM.read(0x06);
	readAddressArray();

	for(uint8_t i=0; i< SETTING_NUM; i++){
		if(SETTING_RULE[i][0]>0){
			if(!checkValue(getValue(i), i)){
				setValue(i, SETTING_RULE[i][0]);
			}
		}
	}
}

void SETTING::save(){
	EEPROM.update(0x00, e.Profile);
	EEPROM.update(0x01, e.Mode);
	EEPROM.update(0x02, (e.HallCenter>>0x08) & 0xFF);
	EEPROM.update(0x03, e.HallCenter);
	EEPROM.update(0x04, e.HallMin);
	EEPROM.update(0x05, (e.HallMax>>0x08) & 0xFF);
	EEPROM.update(0x06, e.HallMax);

	if(CP->NRF.status || e.addr[e.Profile-1]==DEFAULT_ADDR){
		writeAddressArray();
	}
}

short SETTING::getValue(byte index){
	switch(index){
		case 0: return e.Profile; break;
		case 1: return e.Mode; break;

		case 2: return CP->NRF.Rx.BatteryType; break;
		case 3: return CP->NRF.Rx.BatterySerie; break;
		case 4: return CP->NRF.Rx.motorPoles; break;
		case 5: return CP->NRF.Rx.motorPulley; break;
		case 6: return CP->NRF.Rx.wheelPulley; break;
		case 7: return CP->NRF.Rx.wheelSize; break;
		case 8: return CP->NRF.Rx.Stepper; break;
		case 9: return CP->NRF.Rx.SpeedLimit; break;

		case (SETTING_NUM-6): return e.HallCenter; break;
		case (SETTING_NUM-5): return e.HallMin; break;
		case (SETTING_NUM-4): return e.HallMax; break;

		case (SETTING_NUM-2): return Optional; break;
		case (SETTING_NUM-1): return Optional; break;
	}
	return 0;
}

void SETTING::setValue(byte index, short value){
	value = constrain(value, SETTING_RULE[index][1], SETTING_RULE[index][2]);
	switch(index){
		case 0: e.Profile = value; break;
		case 1: e.Mode = value; break;

		case 2: CP->NRF.Rx.BatteryType = value; break;
		case 3: CP->NRF.Rx.BatterySerie = value; break;
		case 4: CP->NRF.Rx.motorPoles = value; break;
		case 5: CP->NRF.Rx.motorPulley = value; break;
		case 6: CP->NRF.Rx.wheelPulley = value; break;
		case 7: CP->NRF.Rx.wheelSize = value; break;
		case 8: CP->NRF.Rx.Stepper = value; break;
		case 9: CP->NRF.Rx.SpeedLimit = value; break;

		case (SETTING_NUM-6): e.HallCenter = value; break;
		case (SETTING_NUM-5): e.HallMin = value; break;
		case (SETTING_NUM-4): e.HallMax = value; break;

		case (SETTING_NUM-2): Optional = value; break;
		case (SETTING_NUM-1): Optional = value; break;
	}
}

bool SETTING::checkValue(uint8_t value, uint8_t index){
	return (value < SETTING_RULE[index][1] || value > SETTING_RULE[index][2]) ? false : true;
}

void SETTING::writeAddressArray(void){
	uint8_t block = 0x07 + ((e.Profile * 0x05)-0x05);

	EEPROM.update(block, (e.addr[e.Profile-1]>>0x20) & 0xFF);
	EEPROM.update(block+0x01, (e.addr[e.Profile-1]>>0x18) & 0xFF);
	EEPROM.update(block+0x02, (e.addr[e.Profile-1]>>0x10) & 0xFF);
	EEPROM.update(block+0x03, (e.addr[e.Profile-1]>>0x08) & 0xFF);
	EEPROM.update(block+0x04, (e.addr[e.Profile-1]));
}

void SETTING::readAddressArray(void){
	uint8_t block = 0x07;

	for(uint8_t i=0; i<PROFILE_NUM; i++){
		e.addr[i]=0x00;
		uint8_t x = 0x00;
		uint8_t shift = 0x28;
		do{
			e.addr[i]+=(uint64_t)EEPROM.read(block+x)<<(shift-=0x08);
			x+=0x01;
		}while(x < 0x05);
		block+=0x05;

		if(!CP->NRF.checkAddress(e.addr[i])){
			e.addr[i]=DEFAULT_ADDR;
		}
	}
}