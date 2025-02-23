#ifndef _TX_SCREEN_H
#define _TX_SCREEN_H
#include <U8g2lib.h>

class SCREEN {
public:
	U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C * U8G2;
	
	SCREEN(void);
	void start(CONTROLLER * CP);
	void update(uint8_t refresh_ms);

private:
	CONTROLLER * CP;

	struct TIMING{
		uint64_t refresh;
		uint64_t delay;
		uint8_t count;
	}Time;

	struct TEMP{
		bool alert;
	}Temp;

	struct DEGREE{
		int16_t total;
		int16_t result;
	}dg;

	void showStartup(void);
	void showMain(void);
	void showSetting(void);
	void drawConnection(uint8_t x, uint8_t y);
	void drawString(String text, uint8_t x, uint8_t y);
	void drawGauge(uint8_t x, uint8_t y, uint8_t r, int16_t input, int16_t inLow, int16_t inHigh, uint16_t strD, uint16_t endD, bool reverse=false);
	uint16_t mapDegree(int16_t input, int16_t inLow, int16_t inHigh, uint16_t strD, uint16_t endD, bool reverse=false);
	uint16_t trigonometryDegree(int16_t degree);
};

#endif