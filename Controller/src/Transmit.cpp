#include "Controller.h"
#include "Transmit.h"

TRANSMIT::TRANSMIT(){
	radio = new RF24(PIN_CE, PIN_CS);
}

bool TRANSMIT::checkNRF24(){
	return radio->isChipConnected();
}

void TRANSMIT::start(CONTROLLER * CP){
	this->CP = CP;

	radio->begin();
	radio->setChannel(DEFAULT_CHANNEL);
	radio->setPALevel(RF24_PA_MAX);
	if(radio->isPVariant()){
		radio->setDataRate(RF24_250KBPS);
	}else{
		radio->setDataRate(RF24_1MBPS);
	}
	radio->enableAckPayload();
	radio->openWritingPipe(CP->ROM.e.addr[CP->ROM.e.Profile-1]);
	radio->setRetries(15,15);
}

void TRANSMIT::send(){
	if(CP->Charge.USB){
		if(!CP->Charge.Standby){
			radio->powerDown();
			CP->Charge.Standby = true;
		}
	}else if(CP->Charge.Standby){
		radio->powerUp();
		CP->Charge.Standby = false;
	}
	
	if((millis()-CP->Time.Lock) > 300){
		switch(Px.type){
		case 1:
			if(radio->write(&Px, sizeof(Px))){
				if(radio->isAckPayloadAvailable()){
					if(radio->getDynamicPayloadSize()==sizeof(VESC)){
						radio->read(&VESC, sizeof(VESC));
						if(VESC.inpVoltage > 12){
							vesc = true;
						}
					}else{
						vesc = false;
					}
				}
				if(ping > 1){
					ping = 0;
					if(!status){
						status = true;
					}
				}
			}
		break;
		case 2:
			if(openpipe > 1){
				openpipe--;
				if(openpipe == 1){
					radio->openWritingPipe(CP->ROM.e.addr[CP->ROM.e.Profile-1]);
					openpipe = 0;
				}
			}
			switch(Tx.type){
				case 2://Reload.
					reloadTx();
				break;
				case 4://Change profile.
					refreshRX();
					reloadTx();
					radio->openWritingPipe(CP->ROM.e.addr[CP->ROM.e.Profile-1]);
				break;
				case 6://Generate address.
				break;
				case 8://Confirm address change.
				break;
				case 10://Reset address.
					Tx.addr = DEFAULT_ADDR;
					CP->ROM.e.addr[CP->ROM.e.Profile-1] = DEFAULT_ADDR;
					openpipe = LIMIT;
					if(Tx.BatteryType>0){
						reloadRx();
					}
				break;
				case 12: //Save and Exit.
					CP->ROM.save();
				break;
				case 120: // Exit without Saving
					reloadTx();
				break;
			}
			
			if(radio->write(&Tx, sizeof(Tx))){
				Tx.type = 0;
				if(ping > 1){
					ping = 0;
					if(!status){
						status = true;
						CP->Menu.msg = 1;
						Tx.type = 103;
						refreshRX();
					}
				}
			}

			if(radio->isAckPayloadAvailable()){
				if(radio->getDynamicPayloadSize() == sizeof(Tx)){
					radio->read(&Tx, sizeof(Tx));
					switch(Tx.type){
						case 3://Reload.
							reloadRx();
							Tx.type = 103;
						break;
						case 5://Change profile.
							reloadRx();
							Tx.type = 105;
						break;
						case 7://Generate address.
							reloadRx();
							Tx.type = 107;
						break;
						case 9://Confrim address change.
							openpipe = LIMIT;
							Tx.type = 109;
						break;
						case 11://Reset address.
							Tx.type = 111;
						break;
						case 13://Save and Exit.
							Tx.type = 113;
						break;
					}
					sent = Tx.type;
				}
			}
		break;
		}
		
		if(!status || CP->Charge.USB){
			CP->Time.Lock = millis();
			if(CP->Charge.USB){
				CP->Menu.msg = 1;
				sent = 20;
			}
		}
	}

	if(ping > LIMIT){
		if(status){
			radio->flush_tx();
			status = false;
			CP->Lock = true;
		}
	}else{
		ping++;
		if(ping > LIMIT){
			if(CP->NRF.Px.type == 2){
				start(CP);
				CP->Menu.msg = 1;
				sent = 21;
			}
		}
	}
}

String TRANSMIT::uint64ToAddress(uint64_t number){
  unsigned long part1 = (unsigned long)(number >> 32);
  unsigned long part2 = (unsigned long)(number);

  return String(part1, HEX) + String(part2, HEX);
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

void TRANSMIT::reloadTx(){
	Tx.BatteryType = Rx.BatteryType;
	Tx.BatterySerie = Rx.BatterySerie ;
	Tx.motorPoles = Rx.motorPoles;
	Tx.motorPulley = Rx.motorPulley;
	Tx.wheelPulley = Rx.wheelPulley;
	Tx.wheelSize = Rx.wheelSize;
	Tx.Stepper = Rx.Stepper;
	Tx.SpeedLimit = Rx.SpeedLimit;
	Tx.addr = CP->ROM.e.addr[CP->ROM.e.Profile-1];
}

void TRANSMIT::reloadRx(){
	Rx.BatteryType = Tx.BatteryType;
	Rx.BatterySerie = Tx.BatterySerie ;
	Rx.motorPoles = Tx.motorPoles;
	Rx.motorPulley = Tx.motorPulley;
	Rx.wheelPulley = Tx.wheelPulley;
	Rx.wheelSize = Tx.wheelSize;
	Rx.Stepper = Tx.Stepper;
	Rx.SpeedLimit = Tx.SpeedLimit;
	CP->ROM.e.addr[CP->ROM.e.Profile-1] = Tx.addr;
}

void TRANSMIT::refreshRX(){
	Rx.BatteryType = 0;
	Rx.BatterySerie = 0;
	Rx.motorPoles = 0;
	Rx.motorPulley = 0;
	Rx.wheelPulley = 0;
	Rx.wheelSize = 0;
	Rx.Stepper = 0;
	Rx.SpeedLimit = 0;
}