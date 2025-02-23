#include "Controller.h"

void CONTROLLER::begin(){
	ROM.load(this);
	OLED.start(this);
	NRF.start(this);

	pinMode(PIN_R100K, INPUT);
	pinMode(PIN_R10K, INPUT);
	pinMode(PIN_TRIGGER, INPUT_PULLUP);
	pinMode(0x08, OUTPUT);
	pinMode(0x07, OUTPUT);
	pinMode(0x06, OUTPUT);

	NRF.Px.trigger = getTrigger() ? 1 : 0;
}

void CONTROLLER::readBattery(void){
	if((millis() - Time.Charge) > 3600){
		Time.Charge = millis();
    	Charge.analog = analogRead(PIN_R100K);
		Battery.analog = analogRead(PIN_R10K);
		
		if(Charge.analog < P_MIN && !Charge.USB){
			digitalWrite(0x08, HIGH);
			digitalWrite(0x07, HIGH);
			digitalWrite(0x06, HIGH);
			Charge.USB = true;
		}else if(Charge.analog > P_MIN && Charge.USB){
			digitalWrite(0x08, LOW);
			digitalWrite(0x07, LOW);
			digitalWrite(0x06, LOW);
			Charge.USB = false;
		}

		if(Charge.USB){
			Battery.voltage = Battery.analog * 5.0f / 1023.0f;
		}else{
			Battery.voltage = Battery.analog * 5.0f / 1023.0f;
		}

		//Battery.percent = (Battery.voltage - BATT_MIN) * (100 / (BATT_MAX - BATT_MIN));
		Battery.percent = map(Battery.voltage, BATT_MIN, BATT_MAX, 0, 100);
  	}
}

void CONTROLLER::controlMain(void){
	if(Lock && millis()-Time.Lock > 1000){
		if(checkNeutral()){
			Lock = false;
		}
}
	
	if (Charge.USB || Lock){
		NRF.Px.mode = 0;
		NRF.Px.trigger = 0;
    	NRF.Px.throttle = NEUTRAL;
    }else{
		NRF.Px.mode = ROM.e.Mode;
		NRF.Px.trigger = getTrigger() ? 1 : 0;
		NRF.Px.throttle = getThrottle();
	}
}

void CONTROLLER::controlSetting(void){
	if(HALL_ANALOG < (ROM.e.HallCenter - MENU_MARGIN)){
		if(Menu.num < (SETTING_NUM-1) && Menu.change == 0){
			Menu.num++;
			Menu.change = 1;
		}else if(Menu.change == 2){
			if(SETTING_RULE[Menu.num][0]>0){
				ROM.setValue(Menu.num, constrain(ROM.getValue(Menu.num)-1, SETTING_RULE[Menu.num][1], SETTING_RULE[Menu.num][2]));
			}else if(Menu.num==(SETTING_NUM-3)){
				NRF.Tx.type = 6;
			}
			Menu.change = 3;
		}
	}else if(HALL_ANALOG > (ROM.e.HallCenter + MENU_MARGIN)){
		if(Menu.num > 0 && Menu.change == 0){
			Menu.num--;
			Menu.change = 1;
		}else if(Menu.change == 2){
			if(SETTING_RULE[Menu.num][0]>0){
				ROM.setValue(Menu.num, constrain(ROM.getValue(Menu.num)+1, SETTING_RULE[Menu.num][1], SETTING_RULE[Menu.num][2]));
			}
			Menu.change = 3;
		}
	}else if(HALL_ANALOG > (ROM.e.HallCenter - MENU_MARGIN) && HALL_ANALOG < (ROM.e.HallCenter + MENU_MARGIN)){
		if(Menu.change == 1){
			Menu.change = 0;
		}else if(Menu.change == 3){
			Menu.change = 2;
		}
	}

	if(getTrigger() && NRF.Px.trigger<1){
		if(Menu.select==0){
			Menu.change=2;
			Menu.select=1;
		}else if(Menu.select==2){
			Menu.change=1;
			Menu.select=3;
		}
		switch(Menu.num){
			case 0://Change profile.
				if(Menu.change==1){
					Menu.msg = 1;
					NRF.Tx.type = 4;
				}
			break;
			case (SETTING_NUM-3)://Change address.
				if(Menu.change==1){
					Menu.msg = 1;
					NRF.Tx.type = 8;
				}
			break;
			case (SETTING_NUM-2)://Reset address.
				if(ROM.getValue(Menu.num)==2){
					Menu.msg = 1;
					NRF.Tx.type = 10;
					ROM.setValue(Menu.num, 1);
				}
			break;
			case (SETTING_NUM-1)://Save and Exit.
				if(ROM.getValue(Menu.num)==2){
					Menu.msg = 1;
					NRF.Tx.type = 12;
					ROM.setValue(Menu.num, 1);
				}else{
					if(Menu.change==1){
						Menu.msg = 1;
						NRF.Tx.type = 120;
					}
				}
			break;
			default: // Reload.
				if(Menu.change==1){
					NRF.Tx.type = 2;
				}
		}
		NRF.sent = NRF.Tx.type;
	}else if(!getTrigger()){
		if(NRF.Px.trigger > 0 && Menu.change<4){
			NRF.Px.trigger=0;
		}
		if(Menu.select==1){
			Menu.select=2;
		}else if(Menu.select==3){
			Menu.select=0;
		}
	}
}

void CONTROLLER::readHallSensor(void){
  HALL_ANALOG = filter(analogRead(PIN_HALL), HALL_ANALOG, 0.5);
}

bool CONTROLLER::getTrigger(void){
	return (!digitalRead(PIN_TRIGGER)) ? true : false;
}

uint16_t CONTROLLER::getThrottle(void){
	if(HALL_ANALOG >= ROM.e.HallCenter){
		return constrain( map(HALL_ANALOG, ROM.e.HallCenter, ROM.e.HallMax, NEUTRAL, P_MAX), NEUTRAL, P_MAX);
	}else{
		return constrain( map(HALL_ANALOG, ROM.e.HallMin, ROM.e.HallCenter, P_MIN, NEUTRAL), P_MIN, NEUTRAL);
	}
}

bool CONTROLLER::checkNeutral(void){
	if(getThrottle()-SAFE_MARGIN < NEUTRAL && getThrottle()+SAFE_MARGIN > NEUTRAL){
		return true;
	}
	return false;
}

uint16_t CONTROLLER::filter( uint16_t newSample, uint16_t oldSample, float alpha ){
	return (uint16_t)((alpha * newSample) + (1.0-alpha) * oldSample);
}