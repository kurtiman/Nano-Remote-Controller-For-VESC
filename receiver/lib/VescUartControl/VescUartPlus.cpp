#include "VescUartPlus.h"
#include <HardwareSerial.h>

VescUart::VescUart(void){
	this->valXJoy         = 127;
	this->valYJoy         = 127;
	this->valLowerButton  	= false;
	this->valUpperButton  	= false;
}

void VescUart::setSerialPort(HardwareSerial* port)
{
	serialPort = port;
}

void VescUart::setDebugPort(Stream* port)
{
	debugPort = port;
}

int VescUart::receiveUartMessage(uint8_t * payloadReceived) {

	// Messages <= 255 starts with "2", 2nd byte is length
	// Messages > 255 starts with "3" 2nd and 3rd byte is length combined with 1st >>8 and then &0xFF

	uint16_t counter = 0;
	uint16_t endMessage = 256;
	bool messageRead = false;
	uint8_t messageReceived[256];
	uint16_t lenPayload = 0;

	uint32_t timeout = millis() + 100; // Defining the timestamp for timeout (100ms before timeout)

	while ( millis() < timeout && messageRead == false) {

		while (serialPort->available()) {

			messageReceived[counter++] = serialPort->read();

			if (counter == 2) {

				switch (messageReceived[0])
				{
					case 2:
						endMessage = messageReceived[1] + 5; //Payload size + 2 for sice + 3 for SRC and End.
						lenPayload = messageReceived[1];
					break;

					case 3:
						// ToDo: Add Message Handling > 255 (starting with 3)
						if( debugPort != NULL ){
							debugPort->println("Message is larger than 256 bytes - not supported");
						}
					break;

					default:
						if( debugPort != NULL ){
							debugPort->println("Unvalid start bit");
						}
					break;
				}
			}

			if (counter >= sizeof(messageReceived)) {
				break;
			}

			if (counter == endMessage && messageReceived[endMessage - 1] == 3) {
				messageReceived[endMessage] = 0;
				if (debugPort != NULL) {
					debugPort->println("End of message reached!");
				}
				messageRead = true;
				break; // Exit if end of message is reached, even if there is still more data in the buffer.
			}
		}
	}
	if(messageRead == false && debugPort != NULL ) {
		debugPort->println("Timeout");
	}

	bool unpacked = false;

	if (messageRead) {
		unpacked = unpackPayload(messageReceived, endMessage, payloadReceived);
	}

	if (unpacked) {
		// Message was read
		return lenPayload;
	}
	else {
		// No Message Read
		return 0;
	}
}


bool VescUart::unpackPayload(uint8_t * message, int lenMes, uint8_t * payload) {

	uint16_t crcMessage = 0;
	uint16_t crcPayload = 0;

	// Rebuild crc:
	crcMessage = message[lenMes - 3] << 8;
	crcMessage &= 0xFF00;
	crcMessage += message[lenMes - 2];

	if(debugPort!=NULL){
		debugPort->print("SRC received: "); debugPort->println(crcMessage);
	}

	// Extract payload:
	memcpy(payload, &message[2], message[1]);

	crcPayload = crc16(payload, message[1]);

	if( debugPort != NULL ){
		debugPort->print("SRC calc: "); debugPort->println(crcPayload);
	}

	if (crcPayload == crcMessage) {
		if( debugPort != NULL ) {
			debugPort->print("Received: ");
			serialPrint(message, lenMes); debugPort->println();

			debugPort->print("Payload :      ");
			serialPrint(payload, message[1] - 1); debugPort->println();
		}

		return true;
	}else{
		return false;
	}
}


int VescUart::packSendPayload(uint8_t * payload, int lenPay) {

	uint16_t crcPayload = crc16(payload, lenPay);
	int count = 0;
	uint8_t messageSend[256];

	if (lenPay <= 256)
	{
		messageSend[count++] = 2;
		messageSend[count++] = lenPay;
	}
	else
	{
		messageSend[count++] = 3;
		messageSend[count++] = (uint8_t)(lenPay >> 8);
		messageSend[count++] = (uint8_t)(lenPay & 0xFF);
	}

	memcpy(&messageSend[count], payload, lenPay);

	count += lenPay;
	messageSend[count++] = (uint8_t)(crcPayload >> 8);
	messageSend[count++] = (uint8_t)(crcPayload & 0xFF);
	messageSend[count++] = 3;
	messageSend[count] = '\0';

	if(debugPort!=NULL){
		debugPort->print("UART package send: "); serialPrint(messageSend, count);
	}

	// Sending package
	serialPort->write(messageSend, count);

	// Returns number of send bytes
	return count;
}


bool VescUart::processReadPacket(uint8_t * message) {

	COMM_PACKET_ID packetId;
	int32_t ind = 0;

	packetId = (COMM_PACKET_ID)message[0];
	message++; // Removes the packetId from the actual message (payload)

	switch (packetId){
		case COMM_GET_VALUES: // Structure defined here: https://github.com/vedderb/bldc/blob/43c3bbaf91f5052a35b75c2ff17b5fe99fad94d1/commands.c#L164
			//ind += 4; // Skip the next 4 bytes
			this->tempFetFiltered   = buffer_get_float16(message, 1e1, &ind);
			this->tempMotorFiltered = buffer_get_float16(message, 1e1, &ind);
			this->avgMotorCurrent 	= buffer_get_float32(message, 1e2, &ind);
			this->avgInputCurrent 	= buffer_get_float32(message, 1e2, &ind);
			ind += 8; // Skip the next 8 bytes
			this->dutyNow 			= buffer_get_float16(message, 1e3, &ind);
			this->rpm 				= buffer_get_float32(message, 1.0, &ind);
			this->inpVoltage 		= buffer_get_float16(message, 1e1, &ind);
			this->ampHours 			= buffer_get_float32(message, 1e4, &ind);
			this->ampHoursCharged 	= buffer_get_float32(message, 1e4, &ind);
			this->wattHours 		= buffer_get_float32(message, 1e4, &ind);
			this->wattHoursCharged 	= buffer_get_float32(message, 1e4, &ind);
			this->tachometer 		= buffer_get_int32(message, &ind);
			this->tachometerAbs 	= buffer_get_int32(message, &ind);
			this->faultCode			= message[ind++];
			this->pidPos			= buffer_get_float32(message, 1e6, &ind);
			this->id					= message[ind++];	
			return true;
		break;

		default:
			return false;
		break;
	}
}

bool VescUart::getVescValues(void) {

	uint8_t command[1] = { COMM_GET_VALUES };
	uint8_t payload[256];

	packSendPayload(command, 1);
	// delay(1); //needed, otherwise data is not read

	int lenPayload = receiveUartMessage(payload);

	if (lenPayload > 55) {
		bool read = processReadPacket(payload); //returns true if sucessful
		return read;
	}
	else
	{
		return false;
	}
}

void VescUart::setNunchuckValues() {
	int32_t ind = 0;
	uint8_t payload[11];

	payload[ind++] = COMM_SET_CHUCK_DATA;
	payload[ind++] = this->valXJoy;
	payload[ind++] = this->valYJoy;
	buffer_append_bool(payload, this->valLowerButton, &ind);
	buffer_append_bool(payload, this->valUpperButton, &ind);

	// Acceleration this-> Not used, Int16 (2 byte)
	payload[ind++] = 0;
	payload[ind++] = 0;
	payload[ind++] = 0;
	payload[ind++] = 0;
	payload[ind++] = 0;
	payload[ind++] = 0;

	if(debugPort != NULL){
		debugPort->println("Data reached at setNunchuckValues:");
		debugPort->print("valXJoy = "); debugPort->print(this->valXJoy); debugPort->print(" valYJoy = "); debugPort->println(this->valYJoy);
		debugPort->print("valLowerButton = "); debugPort->print(this->valLowerButton); debugPort->print(" valUpperButton = "); debugPort->println(this->valUpperButton);
	}

	packSendPayload(payload, 11);
}

void VescUart::setCurrent(float current) {
	int32_t index = 0;
	uint8_t payload[5];

	payload[index++] = COMM_SET_CURRENT;
	buffer_append_int32(payload, (int32_t)(current * 1000), &index);

	packSendPayload(payload, 5);
}

void VescUart::setBrakeCurrent(float brakeCurrent) {
	int32_t index = 0;
	uint8_t payload[5];

	payload[index++] = COMM_SET_CURRENT_BRAKE;
	buffer_append_int32(payload, (int32_t)(brakeCurrent * 1000), &index);

	packSendPayload(payload, 5);
}

void VescUart::setRPM(float rpm) {
	int32_t index = 0;
	uint8_t payload[5];

	payload[index++] = COMM_SET_RPM ;
	buffer_append_int32(payload, (int32_t)(rpm), &index);

	packSendPayload(payload, 5);
}

void VescUart::setDuty(float duty) {
	int32_t index = 0;
	uint8_t payload[5];

	payload[index++] = COMM_SET_DUTY;
	buffer_append_int32(payload, (int32_t)(duty * 100000), &index);

	packSendPayload(payload, 5);
}

void VescUart::serialPrint(uint8_t * data, int len) {
	if(debugPort != NULL){
		for (int i = 0; i <= len; i++)
		{
			debugPort->print(data[i]);
			debugPort->print(" ");
		}

		debugPort->println("");
	}
}

void VescUart::printVescValues() {
	if(debugPort != NULL){
		debugPort->print("tempFetFiltered: "); 		debugPort->println(this->tempFetFiltered);
		debugPort->print("tempMotorFiltered: "); 	debugPort->println(this->tempMotorFiltered);
		debugPort->print("avgMotorCurrent: "); 		debugPort->println(this->avgMotorCurrent);
		debugPort->print("avgInputCurrent: "); 		debugPort->println(this->avgInputCurrent);
		//debugPort->print("avgId: "); 		debugPort->println(this->avgId);
		//debugPort->print("avgIq: "); 		debugPort->println(this->avgIq);
		debugPort->print("dutyNow: "); 				debugPort->println(this->dutyNow);
		debugPort->print("rpm: "); 					debugPort->println(this->rpm);
		debugPort->print("inputVoltage: "); 		debugPort->println(this->inpVoltage);
		debugPort->print("ampHours: "); 			debugPort->println(this->ampHours);
		debugPort->print("ampHoursCharges: "); 		debugPort->println(this->ampHoursCharged);
		//debugPort->print("wattHours: "); 			debugPort->println(this->wattHours);
		//debugPort->print("watthoursCharged: "); 	debugPort->println(this->watthoursCharged);
		debugPort->print("tachometer: "); 			debugPort->println(this->tachometer);
		debugPort->print("tachometerAbs: "); 		debugPort->println(this->tachometerAbs);
	}
}
