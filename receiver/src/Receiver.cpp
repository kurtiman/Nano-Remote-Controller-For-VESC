#include "Receiver.h"

void RECEIVER::begin(void){
	Serial.begin(115200);
	
	#ifdef DEBUG
		while(!Serial){ delay(300); }
	#else
	 	#ifdef NEWER_FW_VESC
		dataUART.setSerialPort(&Serial);
		#else
		SetSerialPort(&Serial);
		#endif
	#endif

	ROM.load(this);
	NRF.start(this);

	#ifdef LED_INDICATOR
	pinMode(PIN_LED, OUTPUT);
	#endif
	pinMode(PIN_PPM, OUTPUT);
	PPM.attach(PIN_PPM);

	calculateRatio();

	VESC.throttle = NEUTRAL;
}

void RECEIVER::commit(void){
	if(NRF.status){
		switch(NRF.Px.mode){
			case 2:
				if(NRF.Px.trigger==1 || VESC.trigger==1){
					modeCruise(true);
				}else{
					modeCruise(false);
				}
			break;
			case 3:
				modeLimit(ROM.e.SpeedLimit);
			break;
		}
	}
	PPM.writeMicroseconds(VESC.throttle);
}

void RECEIVER::checkTrigger(void){
#ifdef LOCK_DC
	if(NRF.Px.trigger==1){
		if(dc.count==2){
			dc.count=3;
			VESC.trigger = 1;
		}else if(dc.count==4){
			dc.count = 5;
		}else if(dc.count<1){
			dc.time = millis();
			dc.count = 1;
		}
	}else if(dc.count==1 || dc.count==3){
		dc.count++;
	}else if(dc.count == 5){
		VESC.trigger = 0;
	}

	if(VESC.trigger<1 && dc.count>1){
		if(((millis() - dc.time)>dc.timeout)){
			dc.count = 0;
		}
	}

	//If disconnected.
	if(!NRF.status && VESC.trigger!=0){
		VESC.trigger = 0;
	}
#else
	VESC.trigger = NRF.Px.trigger;
#endif
}

void RECEIVER::checkThrottle(void){
	#ifdef AUTO_DEACCERELATION
	static long lastRPM;
	#endif

	if(NRF.status){
/*
1st stage safety.
Check if safe.active.
2nd stage safety.
Check if safe.brake.
*/
		if(!safe.active){
			if(checkNeutral()){
				safe.active = true;
			}
		}else{
			if(safe.brake){
				VESC.throttle = NRF.Px.throttle;
			}
		}
	}

/*
If the signal is lost or disconnected.
Apply and check two stages of safety.
- (boolean) safe.active;
- (boolean) safe.brake;
*/ 
	if(!NRF.status || !safe.active || !safe.brake){
		if(!checkNeutral(SAFE_MARGIN, &VESC.throttle)){
			/*
Active 1st stage.
If VESC input is not in neutral.
*/
			if(safe.active){
				safe.active = false;
			}
		}
		#ifdef AUTO_DEACCERELATION
/*
Active 2nd stage.
If VESC speed is above the threshold.
*/		
		if(dataUART.rpm > Cruise.COMPENSATE * 2){
			if(safe.brake){
				safe.brake = false;
			}

			if((millis() - safe.timeout) > (LIMIT_TIMEOUT * ROM.e.Stepper)){
				safe.timeout = millis();
				if(NRF.status){
					if((NRF.Px.throttle >= VESC.throttle-SAFE_MARGIN) && (NRF.Px.throttle  <= VESC.throttle+SAFE_MARGIN)){
						safe.active = true;
						safe.brake = true;
					}else if(NRF.Px.throttle > VESC.throttle){
						VESC.throttle+=ROM.e.Stepper;
					}else if(NRF.Px.throttle < VESC.throttle){
						VESC.throttle-=ROM.e.Stepper;
					}
				}else{
					if(dataUART.rpm > lastRPM || VESC.throttle > NEUTRAL){
						VESC.throttle-=ROM.e.Stepper;
						if(VESC.throttle < P_MIN){
							VESC.throttle = P_MIN;
						}
					}
					lastRPM = dataUART.rpm;
				}                                     
			}
		}else{
			if(!checkNeutral(SAFE_MARGIN, &VESC.throttle)){
				if((millis() - safe.timeout) > (LIMIT_TIMEOUT * ROM.e.Stepper)){
					safe.timeout = millis();
					if(VESC.throttle < NEUTRAL){
						VESC.throttle+=ROM.e.Stepper;
					}else{
						VESC.throttle-=ROM.e.Stepper;
					}
				}
			}else{
				if(!safe.brake){
					safe.brake = true;
				}
			}
		}
		#else
		VESC.throttle = NEUTRAL;
		if(!safe.brake){
			safe.brake = true;
		}
		#endif
	}
}

bool RECEIVER::checkNeutral(uint8_t threshold, uint16_t * input){
	if(input == __null){
		input = &NRF.Px.throttle;
	}
	if(*input - threshold < NEUTRAL && *input + threshold > NEUTRAL){
		return true;
	}
	return false;
}

void RECEIVER::checkVESC(void){
	static uint8_t i = 0;
	static float TotalavgInputCurrent = 0.0;
	static float TotalavgSpeed = 0.0;
	uint8_t max = 25;

	#ifdef NEWER_FW_VESC
	if(dataUART.getVescValues())
	#else
	if(VescUartGetValue(dataUART))
	#endif
	{
		if(i>=max){
			VESC.avgInputCurrent = filter((int8_t)round((TotalavgInputCurrent/max) * 10), (int8_t)VESC.avgInputCurrent, 0.5);
			VESC.speed = filter((float)TotalavgSpeed/max, (float) VESC.speed, 0.5);
			TotalavgInputCurrent = 0;
			TotalavgSpeed = 0;
			i = 0;
		}else{
			TotalavgInputCurrent += dataUART.avgInputCurrent;
			TotalavgSpeed += (r.RPM * dataUART.rpm);
			i++;
		}

		VESC.tempFetFiltered	=	round(dataUART.tempFetFiltered);
		VESC.inpVoltage			=	dataUART.inpVoltage;
		VESC.ampHours			=	round((dataUART.ampHours * 10));
	 	VESC.ampHoursCharged	=	round((dataUART.ampHoursCharged * 10));

	    VESC.distance	=	(r.DIST * dataUART.tachometerAbs);
		dataUART.rpm 	=	(dataUART.rpm * 0.142857);
    }else{
    	VESC.speed = 0;
    }
}

void RECEIVER::calculateRatio(void){
  r.GEAR  = ((float)ROM.e.motorPulley / (float)ROM.e.wheelPulley);
  r.RPM = ((r.GEAR * 60) * ((float)ROM.e.wheelSize * PI)) / (((float)ROM.e.motorPoles * 0.5) * 1000000);
  r.DIST =((r.GEAR * ((float)ROM.e.wheelSize * PI)) / (((float)ROM.e.motorPoles * 3) * 1000000) * 0.825);
}

#ifdef LED_INDICATOR
void RECEIVER::LedIndicator(void){
	static uint8_t standby;
	if(!NRF.status){
		if(led.rate != 5){
			led.rate = 5;
		}
		if(millis() - led.cycle > (uint16_t) led.rate){
			led.cycle = millis();
			if(led.current == ON){
				standby+=led.rate;
			}else{
				standby--;
			}

			if(standby >= 250){
				led.current = OFF;
			}else if(standby <= led.rate){
				led.current = ON;
			}
			analogWrite(PIN_LED, standby);
		}
	}else{
		if(led.rate!=50){
			led.rate = 50;
		}
		if(millis() - led.cycle > (uint16_t) led.rate){
			led.cycle = millis();
			if(led.current == OFF){
				led.current = ON;
			}else{
				led.current = OFF;
			}
			digitalWrite(PIN_LED, led.current);
		}
	}
}
#endif

void RECEIVER::modeCruise(bool active){
	if(safe.active && safe.brake){
		if(active){
			switch(Cruise.init){
				case 0:
					Cruise.init = 1;
					Cruise.set = dataUART.rpm + Cruise.COMPENSATE;
					Cruise.setHold = Cruise.set;
					Cruise.Kp = (0.01 * Cruise.maxP);
				break;
				case 1:
					if(NRF.Px.trigger==0){
						Cruise.init = 2;
					}
					if(!Cruise.stamp){
						Cruise.stamp = true;
					}
				break;
			}

/*
If the trigger switch is not held down and double click is enabled and actived.
Allows cruise input adjustment by the throttle input.
But if the throttle input exceeds the threshold, de-activate the cruise control.
*/			
			if(NRF.Px.trigger==0){
				if(!checkNeutral() && Cruise.init==2){
					/*Added Sensitive threshold to your needs. Default: 150*/
					if(!checkNeutral(NEUTRAL_MARGIN)){
						VESC.trigger = 0;
						Cruise.Recalculate(0);
					}else if(NRF.Px.throttle > NEUTRAL){
						/*Added delays to avoid surges.*/
						if(millis() - Cruise.time > LIMIT_TIMEOUT){
							Cruise.time = millis();
							Cruise.setHold++;
						}
					}else if(NRF.Px.throttle < NEUTRAL){
						if(millis() - Cruise.time > LIMIT_TIMEOUT){
							Cruise.time = millis();
							Cruise.setHold--;
						}
					}
				}
/*
If trigger is holding.
Allows to disengage the cruise and applies brake.
*/
			}else{
				if(!checkNeutral() && NRF.Px.throttle < NEUTRAL){
					Cruise.Recalculate(0);
				}
			}

		}else{
			switch(Cruise.init){
				case 1:
					if(Cruise.stamp){
						Cruise.time = millis();
						Cruise.stamp = false;
					}else if(millis() - Cruise.time > dc.timeout){
						Cruise.Recalculate(0);
					}
				break;
				case 2:
					Cruise.init = 1;
				break;
			}
		}
/*
If cruising on ascend or descend ground.
Compensate the cruise input to maintain speed.
*/
		if(Cruise.init > 0){
			Cruise.Recalculate(dataUART.rpm);
			VESC.throttle = Cruise.output;
		}
	}else{
		Cruise.Recalculate(0);
	}
}

void RECEIVER::modeLimit(uint8_t limit){
	static uint16_t maxRPM = round((limit/r.RPM)/Cruise.REDUNDANCY)+Cruise.COMPENSATE;

	Cruise.Kp = map(NRF.Px.throttle, NEUTRAL, P_MAX, 1, Cruise.maxP);
	
	if(Cruise.Kp == Cruise.maxP){
		if(Cruise.setHold!=maxRPM){
			Cruise.setHold = maxRPM;
		}
	}else{
		if(Cruise.set!=maxRPM){
			Cruise.setHold = 0;
			Cruise.set = maxRPM;
		}
	}

	Cruise.Kp = 0.01 * Cruise.Kp;
	Cruise.Recalculate(dataUART.rpm+1);

	if(NRF.Px.throttle > Cruise.output){
		VESC.throttle = Cruise.output;
	}
}

int8_t RECEIVER::filter( int8_t newSample, int8_t oldSample, float alpha){
	return (int8_t)((alpha * newSample) + ((1.0-alpha) * oldSample));
}
float RECEIVER::filter( float newSample, float oldSample, float alpha){
	return (float)((alpha * newSample) + ((1.0-alpha) * oldSample));
}