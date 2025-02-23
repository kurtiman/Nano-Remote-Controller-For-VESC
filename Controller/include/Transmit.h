#ifndef _TX_TRANSMIT_H
#define _TX_TRANSMIT_H
#include <RF24.h>
#include <nRF24L01.h>

class TRANSMIT{
public:
	DATA_VESC VESC;
	DATA_PACKET Px;
	DATA_TX Tx;
	DATA_RX Rx;

	bool status = false;
	bool vesc = false;
	uint8_t ping;
	uint8_t sent;

	TRANSMIT(void);
	void start(CONTROLLER * CP);
	bool checkNRF24(void);
	void send(void);
	String uint64ToAddress(uint64_t number);
	bool checkAddress(uint64_t addr);
	void refreshRX(void);

private:
	CONTROLLER * CP;
	RF24 * radio;

	uint8_t openpipe;

	void reloadRx(void);
	void reloadTx(void);
};

#endif