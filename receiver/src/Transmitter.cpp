#include "Receiver.h"

TRANSMIT::TRANSMIT(){
	radio = new RF24(PIN_CE, PIN_CS);
}

bool TRANSMIT::checkNRF24(){
	return radio->isChipConnected();
}

void TRANSMIT::start(RECEIVER * RP){
	this->RP = RP;

	checkAddress(RP->ROM.e.addr);
	radio->begin();
	radio->setChannel(DEFAULT_CHANNEL);
	radio->setPALevel(RF24_PA_MAX);
	if(radio->isPVariant()){
		radio->setDataRate(RF24_250KBPS);
	}else{
		radio->setDataRate(RF24_1MBPS);
	}
	radio->enableAckPayload();
	radio->openReadingPipe(0x00, RP->ROM.e.addr);
	radio->startListening();
}

void TRANSMIT::readPacket(){
	#ifdef AUTO_SWITCH_PAIRING
	if(!status){
		if(millis()-ap.timestamp > AUTO_PAIR_INTERVAL){
			radio->flush_rx();
			if(!ap.normal){
				radio->openReadingPipe(0x00, DEFAULT_ADDR);
			}else{
				radio->openReadingPipe(0x00, RP->ROM.e.addr);
			}
			ap.normal=!ap.normal;
			ap.timestamp = millis();
		}
	}else{
		ap.timestamp = millis();
	}
	#endif

	if(openpipe > 1){
		openpipe--;
		if(openpipe == 1){
			radio->openReadingPipe(0x00, RP->ROM.e.addr);
			openpipe = 0;
		}
	}

	if(radio->available()){
		switch(radio->getDynamicPayloadSize()){
			case sizeof(Px):
				radio->read(&Px, sizeof(Px));
				radio->writeAckPayload(0x00, &RP->VESC, sizeof(RP->VESC));
				if(ping > 1){
					ping = 0;
					if(!status){
						status = true;
					}
				}
			break;
			case sizeof(Tx):
				Tx.type=0;
				radio->read(&Tx, sizeof(Tx));

				if(Tx.type > 100){
					switch(Tx.type){
						case 103:
							if(!checkAddress(Tx.addr)){
								reloadTx();
								Tx.type = 2;
							}else{
								reloadRx();
							}
							RP->calculateRatio();
						break;
						case 105:
							Tx.type = 2;
						break;
						case 107:
							reloadRx();
						break;
						case 109:
							openpipe = LIMIT_TIMEOUT;
						break;
						case 111:
							openpipe = LIMIT_TIMEOUT;
							reloadRx();
						break;
						case 113:
							RP->ROM.save();
						break;
						case 120:
							reloadRx();
						break;
					}
				}

				switch(Tx.type){
					case 2:
						if(checkAddress(Tx.addr)){
							reloadRx();
						}else{
							reloadTx();
						}
						Tx.type = 3;
					break;
					case 4:
						reloadTx();
						Tx.type = 5;
					break;
					case 6:
						do{
							Tx.addr=generateAddress();
						}while(!checkAddress(Tx.addr));
						Tx.type = 7;
					break;
					case 8:
						Tx.type = 9;
					break;
					case 10:
						Tx.addr = DEFAULT_ADDR;
						Tx.type = 11;
					break;
					case 12:
						Tx.type = 13;
					break;
				}

				if(Tx.type > 1 && Tx.type < 20){
					radio->writeAckPayload(0x00, &Tx, sizeof(Tx));
					Tx.type=0;
				}

				if(ping > 1){
					ping = 0;
					if(!status){
						status = true;
					}
				}
			break;
		}
		
	}else{
		if(ping > LIMIT_TIMEOUT){
			if(status){
				radio->flush_rx();
				status = false;
			}
		}else{
			ping++;
		}
	}
}

uint64_t TRANSMIT::StringToUint64(char * string){
	uint64_t x = 0;
	char c;
	do{
		if(isdigit(*string++)){
			c = (*string) - '0';
		}else if(isxdigit(*string)){
			c = ((*string) & 0xF) + 9;
		}else{
			c = -1;
		}
		if(c<0){
			break;
		}
		x = (x << 4) | c;
	}while(1);
	return x;
}

uint64_t TRANSMIT::generateAddress(){
	randomSeed(millis());
	char x[10];
	const char *hex = "0123456789ABCDEF";
	for (uint8_t i = 0 ; i < 10; i++ ){
		if((i % 2)==1){
			x[i] = hex[random(0, 10)];
		}else{
			x[i] = hex[random(11, 16)];
		}
	}
	return (uint64_t) StringToUint64(x);
}

void TRANSMIT::reloadTx(){
	Tx.BatteryType = RP->ROM.e.BatteryType;
	Tx.BatterySerie = RP->ROM.e.BatterySerie;
	Tx.motorPoles = RP->ROM.e.motorPoles;
	Tx.motorPulley = RP->ROM.e.motorPulley;
	Tx.wheelPulley = RP->ROM.e.wheelPulley;
	Tx.wheelSize = RP->ROM.e.wheelSize;
	Tx.Stepper = RP->ROM.e.Stepper;
	Tx.SpeedLimit = RP->ROM.e.SpeedLimit;
	Tx.addr = RP->ROM.e.addr;
}

void TRANSMIT::reloadRx(){
	RP->ROM.e.BatteryType = Tx.BatteryType;
	RP->ROM.e.BatterySerie = Tx.BatterySerie;
	RP->ROM.e.motorPoles = Tx.motorPoles;
	RP->ROM.e.motorPulley = Tx.motorPulley;
	RP->ROM.e.wheelPulley = Tx.wheelPulley;
	RP->ROM.e.wheelSize = Tx.wheelSize;
	RP->ROM.e.Stepper = Tx.Stepper;
	RP->ROM.e.SpeedLimit = Tx.SpeedLimit;
	RP->ROM.e.addr = Tx.addr;
}

bool TRANSMIT::checkAddress(uint64_t addr){
	uint8_t x;
	for(uint8_t i=1; i<=5; i++){
		x = addr >> (40-(8*i));
		if(x==0x00 || x==0xff){
			return false;
		}
	}
	return true;
}