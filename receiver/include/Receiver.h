#ifndef _RX_H
#define _RX_H
#include <Servo.h>

#include "Config.h"
#include "Transmitter.h"
#include "Setting.h"

#ifdef NEWER_FW_VESC
#include <VescUartPlus.h>
#else
#include <VescUart.h>
#endif

class RECEIVER {
public:
	SETTING ROM;
	TRANSMIT NRF;
	DATA_VESC VESC;

	void begin(void);
	void commit(void);
	void checkVESC(void);
	void checkTrigger(void);
	void checkThrottle(void);
	bool checkNeutral(uint8_t threshold =SAFE_MARGIN, uint16_t * input= __null);
	void calculateRatio(void);

	#ifdef LED_INDICATOR
	void LedIndicator(void);
	#endif

private:
	Servo PPM;
	
	#ifdef NEWER_FW_VESC
	class VescUart dataUART;
	#else
	struct bldcMeasure dataUART;
	#endif
	
	struct SAFETY{
		bool active = false;
		bool brake = false;
		uint64_t timeout;
	} safe;

	struct RATIO{
		float GEAR;
		float RPM;
		float DIST;
	} r;
	
	struct DOUBLECLICK{
		uint64_t time;
		uint8_t count;
		uint16_t timeout = 450;
	} dc;

	#ifdef LED_INDICATOR
	typedef enum{
		ON = 0x1,
		OFF = 0x0
	}PIN_SWITCH;

	struct LED{
		uint64_t cycle;
		int16_t rate;
		PIN_SWITCH current = OFF;
	} led;
	#endif

/*
Cruise Control PID.
*/
	struct CRUISE{
		uint8_t maxP = 31;
		float Kp = (maxP * 0.01);
		float Ki = 0.000001;
		float Kd = 4.0;
		uint8_t init = 0;
		uint8_t REDUNDANCY = 7;
		uint16_t COMPENSATE = 285;
		uint16_t set = 0;
		uint16_t setHold = 0;
		uint16_t output;
		int setError;
		int err = 0;
		int errA = 0;
		int errP = 0;
		int errD = 0;
		uint64_t time = 0;
		bool stamp;
		void Recalculate(int rpm){
			if(rpm==0){
				init = setHold = set = err = errA = errP = errD = rpm;
			}else{
				if(setHold > COMPENSATE){
					setError = (rpm + COMPENSATE) - setHold;
					if(setError < REDUNDANCY){
						set++;
					}else if(setError > REDUNDANCY){
						set--;
					}
				}

				err = (rpm-set);
				if((errA+err) > 0x7FFFFFFF){
					errA = err;
				}else{
					if(output > P_MIN && output < P_MAX){
						errA += err;
					}
				}
				errD = err - errP;
				errP = err;
				output = constrain(NEUTRAL-((Kp*err)+(Ki*errA)+(Kd*errD)), P_MIN, P_MAX);
			}
		}
	} Cruise;

	void modeCruise(bool active = false);
	void modeLimit(uint8_t limit);
	int8_t filter(int8_t newSample, int8_t oldSample, float alpha);
	float filter(float newSample, float oldSample, float alpha);
};

#endif