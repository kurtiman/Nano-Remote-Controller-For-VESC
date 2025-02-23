#ifndef _TX_H
#define _TX_H

#include "Config.h"
#include "Lang.h"
#include "Setting.h"
#include "Screen.h"
#include "Transmit.h"

class CONTROLLER{
public:
	SETTING ROM;
	SCREEN OLED;
	TRANSMIT NRF;
	
	bool Lock = true;
	uint16_t HALL_ANALOG = 512;

	struct CHARGE{
		bool Standby = false;
		bool USB = false;
		uint16_t analog;
	}Charge;

	struct BATTERY{
		uint16_t analog;
		float percent;
		float voltage;
	}Battery;

	struct MENU{
		uint8_t num;
		uint8_t change;
		uint8_t select;
		uint8_t msg;
	}Menu;

	struct TIMING{
		uint64_t Charge;
		uint64_t Lock;
	}Time;

	void begin(void);
	void readBattery(void);
	void readHallSensor(void);
	bool getTrigger(void);
	uint16_t getThrottle(void);
	bool checkNeutral(void);
	void controlMain(void);
	void controlSetting(void);

	uint16_t SET_P = 0;
	uint16_t SET_I = 0;
	uint16_t SET_D = 0;

private:
	uint16_t filter( uint16_t newSample, uint16_t oldSample, float alpha);
};

#endif