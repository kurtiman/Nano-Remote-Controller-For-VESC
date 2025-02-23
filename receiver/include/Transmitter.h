#ifndef _RX_TRANSMIT_H
#define _RX_TRANSMIT_H
#include <RF24.h>

class TRANSMIT{
public:
	DATA_TX Tx;
	DATA_PACKET Px;

	bool status = true;

	TRANSMIT(void);
	void start(RECEIVER * RP);
	void readPacket(void);
	bool checkAddress(uint64_t addr);
	bool checkNRF24(void);

private:
	RF24  * radio;
	RECEIVER * RP;

	uint16_t openpipe;
	uint8_t ping;

	#ifdef AUTO_SWITCH_PAIRING
	struct AP{
		bool normal = false;
		uint64_t timestamp;
	} ap;
	#endif
	
	void start(void);
	uint64_t StringToUint64(char * string);
	uint64_t generateAddress(void);
	void reloadRx(void);
	void reloadTx(void);
};

#endif