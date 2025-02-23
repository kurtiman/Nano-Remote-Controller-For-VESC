#include "Controller.h"

SCREEN::SCREEN(){
	U8G2 = new U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C(U8G2_R1, U8X8_PIN_NONE);
}

void SCREEN::start(CONTROLLER * CP){
	this->CP = CP;
	U8G2->begin();
}

void SCREEN::update(uint8_t refresh_ms){
	if((millis()-Time.refresh) > refresh_ms){
		Time.refresh = millis();
		U8G2->firstPage();
		do{
			if(Time.refresh < 4000){
				CP->Lock = true;
				CP->Time.Lock = millis();
				U8G2->setDisplayRotation(U8G2_R2);
				SCREEN::showStartup();
			}else{
				switch(CP->NRF.Px.type){
					case 2:
						if(CP->Menu.msg==1){
							if(CP->Menu.change < 4){
								U8G2->setFont(u8g2_font_profont12_tr);
								CP->Menu.change=4;
								Time.delay=millis();
								CP->HALL_ANALOG = CP->ROM.e.HallCenter;
								CP->NRF.Px.trigger = 1;
							}else{
								if((millis()-Time.delay)>4000){
									CP->Menu.change = 0;
									CP->Menu.msg = 0;
									if(CP->NRF.sent>=113 || CP->NRF.sent==12){
										CP->NRF.Px.type = 1;
									}
								}
							}
							switch(CP->NRF.sent){
								case 4:
									drawString(feedback_msg_4, 0, 12);
									drawString(CP->NRF.uint64ToAddress(CP->ROM.e.addr[CP->ROM.e.Profile-1]), 45, 30);
								break;
								case 8:
									if(CP->NRF.status){
										drawString(feedback_msg_1, 0, 20);
									}else{
										drawString(feedback_msg_2, 0, 20);
									}
								break;
								case 10:
									drawString(feedback_msg_10, 0, 20);
								break;
								case 12:
									drawString(feedback_msg_12, 0, 20);
								break;
								case 103:
									drawString(feedback_msg_connected, 0, 24);
									drawConnection(0, 0);
								break;
								case 105:
									drawString(feedback_msg_connected, 0, 24);
									drawConnection(0, 0);
								break;
								case 109:
									drawString(feedback_msg_change_addr, 0, 12);
									drawString(feedback_rx_confirmed, 18, 30);
								break;
								case 111:
									drawString(feedback_msg_reset_addr, 0, 12);
									drawString(feedback_rx_confirmed, 18, 30);
								break;
								case 113:
									drawString(feedback_msg_save, 0, 12);
									drawString(feedback_rx_synced, 18, 30);
								break;
								case 120:
									drawString(feedback_msg_exit, 0, 12);
									drawString(feedback_not_saved, 18, 30);
								break;
								case 20:
									drawString(feedback_msg_off, 0, 20);
								break;
								case 21:
									drawString(feedback_msg_disconnected, 0, 24);
									drawConnection(0, 0);
								break;
								default:
									drawString("Status Feedback : "+String(CP->NRF.sent), 0, 20);
							}
						}else{
							showSetting();
						}
					break;
					default:
						U8G2->setDisplayRotation(U8G2_R1);
						SCREEN::showMain();
				}
			}
		}while(U8G2->nextPage());
	}
}

void SCREEN::showStartup(void){
	static int8_t x = 120;
	static int8_t y = 0;
	if(x > 0 && Time.refresh < 2000){ x-=4; }
	switch(CP->NRF.Px.type){
		case 1:
			if(Time.refresh > 3600){ y-=3; }
			U8G2->drawXBMP(x+0, y+0, 31, 31, startupIcon);
			U8G2->setFont(u8g2_font_profont12_tr);
			drawString("Ride Safe!", x+50, y+12);
			drawString("deakbannok", x+50, y+32);
		break;
		case 2:
			if(x < 125 && Time.refresh > 3600){ x+=5; }
			U8G2->drawXBMP(x+0, 6, y+20, 20, settingIcon);
			U8G2->setFont(u8g2_font_helvR10_tr);
			drawString("Settings", x+30, y+24);
		break;
	}
}

void SCREEN::showMain(void){
	static int8_t mx = -54;
	uint8_t i, x, y, z;
	int8_t span, first, last;
	
	if(mx<0){ mx+=3;}
	
	x = 6+mx;
	y = 0;
	if(!CP->Charge.USB){
		//Display gauge for InputCurrent at max of 50.
		U8G2->drawXBMP(x, y, 22, 17, gaugeIcon);
		if(CP->NRF.VESC.avgInputCurrent >= 0){
			drawGauge(x+10, y+11, 8, CP->NRF.VESC.avgInputCurrent, 1, 50, 270, 90);
		}else{
			drawGauge(x+10, y+11, 8, CP->NRF.VESC.avgInputCurrent, 1, 50, 90, 270);
		}
	}

	x = x-6;
	y = y+34;
	//Speed in Km
	if(CP->NRF.VESC.speed < 0){
		CP->NRF.VESC.speed *= -1;
	}
	first = abs(floor(CP->NRF.VESC.speed));
	last = (CP->NRF.VESC.speed-first)*100;
	drawString((String)last, x+18, y-3);
	U8G2->setFont(u8g2_font_helvR10_tr);
	drawString(((first <= 9) ? "0"+(String)first : (String)first), x, y);
	
	x = x+8;
	y = y+2;
	if((CP->NRF.Px.trigger<1 && CP->NRF.VESC.trigger<1) || !CP->NRF.status){
		U8G2->drawXBMP(x, y, 15, 7, switchOffIcon);
	}else{
		U8G2->drawXBMP(x, y, 15, 7, switchOnIcon);
	}

	x = x-8;
	y = y+10;
	//Battery Board
	U8G2->setFont(u8g2_font_profont12_tr);
	drawString((String)CP->NRF.VESC.inpVoltage, x+2, y + 10);

	y = y+14;
	//Connection status
	drawConnection(x, y);

	x = x+12;
	y = y+5;
	//Battery Remote
	U8G2->drawFrame(x, y - 4, 18, 9);
	uint8_t P;
	for(i = 0; i < 8; i++){
		P = round((100 / 8) * i);
		if (P < CP->Battery.percent){
			U8G2->drawBox(x+ 2 + (2 * i), y - 2, 1, 5);
		}
	}

	x = x-2;
	y = y+10;
	//FET temperature
	U8G2->drawHLine(x-3, y, 7);
	U8G2->drawVLine(x+3, y, 49);
	U8G2->drawHLine(x-3, y+48, 7);

	//If temperature reachs 80C
	first = CP->NRF.VESC.tempFetFiltered;
	span = map(first, 1, 80, 0, 38);
	if(span>38){
		span = 38;
		if(millis()-Time.delay>300){
			Time.delay=millis();
			Temp.alert = !Temp.alert;
		}
	}else{
		Temp.alert=false;
	}

	//Display temperature non-solid graph
	for(i=0; i<span; i++){
		if(!Temp.alert){
			if((i & 1) == 0){
				U8G2->drawHLine(x-10, y+47-i, 12);
			}
		}else if(Temp.alert){
			U8G2->drawHLine(x-10, y+47-i, 12);
		}
	}
	if(first > 0){
		if(first < 10){
			drawString((String)first, x-4, y+47-span);
		}else{
			drawString((String)first, x-10, y+47-span);
		}
	}

	x = x+9;
	//Throttle
	U8G2->drawVLine(x, y, 49);
	U8G2->drawHLine(x+1, y, 4);
	U8G2->drawHLine(x+1, y+48, 4);
	U8G2->drawHLine(x+1, y + 24, 2);
	if(CP->NRF.vesc){
		CP->NRF.Px.throttle = CP->NRF.VESC.throttle;
	}
	if(CP->NRF.Px.throttle >= NEUTRAL) {
    	span = map(CP->NRF.Px.throttle, NEUTRAL, P_MAX, 0, 49);
    	for(i=0; i<span; i++){
				z = map(i, 0, 50, 4, 12);
				U8G2->drawHLine(x+2, y+48 -i, z);
    	}
	}else{
			span = map(CP->NRF.Px.throttle, P_MIN, NEUTRAL-1, 49, 0);
    	for(i=0; i<span; i++){
				z = map(i, 0, 50, 4, 12);
				U8G2->drawHLine(x+2, y+i, z);
    	}
    }
}

void SCREEN::showSetting(void){
	static int8_t mx = -120;
	static uint8_t lastSeen;
	uint8_t x = 0+mx;
	uint8_t y = 10;

	if(lastSeen != CP->Menu.num){
		if(lastSeen > CP->Menu.num){
			mx = 120;
		}else{
			mx = -120;
		}
		lastSeen = CP->Menu.num;
	}
	if(mx < 0){
		mx+=6;
	}else if(mx > 0){
		mx-=6;
	}

	U8G2->setFont(u8g2_font_profont12_tr);

	if(CP->HALL_ANALOG < 10){
		x = x+112;
	}else if(CP->HALL_ANALOG < 100){
		x = x+106;
	}else{
		x = x+100;
	}
	drawString("("+(String)CP->HALL_ANALOG+")", x, y);

	x -= x;
	x +=mx;

	if(CP->Menu.num>=(SETTING_NUM-3)){
		drawConnection(x, y-10);
		drawString(SETTING_TITLE[CP->Menu.num], x+15, y);
	}else{
		drawString(SETTING_TITLE[CP->Menu.num], x, y);
	}

	String value;
	switch(CP->Menu.num){
		case 0:
			value = PROFILE_NAME[(CP->ROM.getValue(CP->Menu.num)-1)];
		break;
		case 1:
			switch(CP->ROM.getValue(CP->Menu.num)){
				case 1: value = menu_mode1; break;
				case 2: value = menu_mode2; break;
				case 3: value = menu_mode3; break;
				default: value = menu_error;
			}
		break;
		case 2:
			switch(CP->ROM.getValue(CP->Menu.num)){
				case 1: value = menu_batt1; break;
				case 2: value = menu_batt2; break;
				default: value = menu_error;
			}
		break;
		case (SETTING_NUM-3):
			if(CP->NRF.checkAddress(CP->NRF.Tx.addr)){
				value = CP->NRF.uint64ToAddress(CP->NRF.Tx.addr);
			}else{
				value = CP->NRF.uint64ToAddress(CP->ROM.e.addr[CP->ROM.e.Profile-1]);
			}
		break;
		case (SETTING_NUM-2):
			switch(CP->ROM.getValue(CP->Menu.num)){
				case 2: value = menu_yes; break;
				default: value = menu_no; break;
			}
		break;
		case (SETTING_NUM-1):
			switch(CP->ROM.getValue(CP->Menu.num)){
				case 2: value = menu_yes; break;
				default: value = menu_no; break;
			}
		break;
		default:
			value = CP->ROM.getValue(CP->Menu.num);
	}
	
	if(CP->Menu.change >= 2){
		drawString(": "+value, x, y+20);
	}else{
		drawString(value, x, y+20);
	}
}

void SCREEN::drawConnection(uint8_t x, uint8_t y){
	if(CP->NRF.status == false){
		if(Time.count > 40 && Time.count < 70){
			U8G2->drawXBMP(x, y, 10, 10, connectedIcon);
		}else if(Time.count > 30 && Time.count < 80) {
			U8G2->drawXBMP(x, y, 10, 10, ConnectHigh);
		}else if(Time.count > 20 && Time.count < 90) {
			U8G2->drawXBMP(x, y, 10, 10, ConnectLow);
		}else{
			U8G2->drawXBMP(x, y, 10, 10, NotConnect);
		}
		if(Time.count>100){
			Time.count=0;
		}
		Time.count++;
	}else{
		U8G2->drawXBMP(x, y, 10, 10, connectedIcon);
	}
}

void SCREEN::drawString(String text, uint8_t x, uint8_t y){
	static char drawString_buffer[32];
	text.toCharArray(drawString_buffer, text.length()+1);
	U8G2->drawStr(x, y, drawString_buffer);
}

void SCREEN::drawGauge(uint8_t x, uint8_t y, uint8_t r, int16_t input, int16_t inLow, int16_t inHigh, uint16_t strD, uint16_t endD, bool reverse){

	if(mapDegree(input, inLow, inHigh, strD, endD, reverse)){
		dg.result = trigonometryDegree(dg.result);
		for(uint8_t i=0; i<6; i++){
			if(i % 2 == 1){
				float rad = ((dg.result+i)*PI)/180;
				U8G2->drawLine(x+((r/2.8) * cos(rad)), y-((r/2.8) * sin(rad)), x+(r * cos(rad)), y-(r *sin(rad)));
			}
		}
	}
}

uint16_t SCREEN::mapDegree(int16_t input, int16_t inLow, int16_t inHigh, uint16_t strD, uint16_t endD, bool reverse){

	if(strD > 360 or endD > 360 or strD < 1 or endD < 1){
        return false;
	}else{
        if(reverse){
            if(strD > endD){
                dg.total = (360 - endD) - (360 - strD);
			}else{
                dg.total = (360 - (360 - strD)) + (360 - endD);
			}
        }else{
            if(strD > endD){
                dg.total = (360 - strD) + endD;
			}else{
                dg.total = strD - endD;
			}
		}

        if(dg.total < 0){
            dg.total *= -1;
		}
	}
	if(input > inHigh){ input = inHigh; }
	if(input < inLow){ input = inLow; }
	input = (inHigh - inLow)-(inHigh-input);
    dg.result = ((float)dg.total/(inHigh - inLow)) * input;

    if(!reverse){
        dg.result = strD + dg.result;
    }else{
        dg.result = strD - dg.result;
	}

    if(dg.result > 360){
        dg.result = (360 - dg.result+1) * (-1);
        if(dg.result<1){
            dg.result = 360;
		}
	}else if(dg.result < 1){
        dg.result = (360 + dg.result);
        if(dg.result > 360){
            dg.result = 360;
		}
	}
	
    return dg.result;
}

uint16_t SCREEN::trigonometryDegree(int16_t degree){	
	static uint8_t multiply;
	
	if(degree > 90 && degree > (90 * multiply)){
		multiply++;
	}
	if(degree > 90){
		degree = (180+(90* multiply)) - (degree - (270-(90 * multiply)));
	}else{
		degree = (90 - degree);
		multiply = 0;
	}
	return degree;
}