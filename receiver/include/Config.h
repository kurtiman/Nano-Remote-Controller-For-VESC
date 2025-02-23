#ifndef _RX_CONFIG_H
#define _RX_CONFIG_H
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

class RECEIVER;

/* Update new PINs for a custom PCB
- A0
- A1
- D2
- D3
- D6 (LED status)
*/

/* Newer firmware 3.4+ VESC. */
//#define NEWER_FW_VESC

/* Arduino NANO with NRF24 built-in. */
//#define BUILT_IN_NRF24
#ifdef BUILT_IN_NRF24
const uint8_t PIN_CE 		    = 0x0a;
const uint8_t PIN_CS 		    = 0x09;
#else
const uint8_t PIN_CE 		    = 0x09;
const uint8_t PIN_CS 		    = 0x0a;
#endif

/* ENABLE or DISABLE to lock-on double click.
Suitable for cruise mode. To lock-on the set speed.
*/
#define LOCK_DC

/***EXPERIMENT MODE***/
/* ENABLE or DISABLE safe throttle.
In case of signal loss from the Controller.
The Receiver will gradually bring the throttle input to the NEUTRAL position.
If DISABLE, the throttle input will reset to NEUTRAL position.
*/
#define AUTO_DEACCERELATION

/*LED indicator.
Requires 1K-2K ohm resistor on A3 pin.
*/
#define LED_INDICATOR
#ifdef LED_INDICATOR
const uint8_t PIN_LED = 0x06;
#endif

/* Auto switch pairing between default and the assigned ADDR.
Useful in re-pairing a new controller without resetting the Receiver's ADDR.
*/
#define AUTO_SWITCH_PAIRING
#ifdef AUTO_SWITCH_PAIRING
const uint16_t AUTO_PAIR_INTERVAL = 15000; // 15 seconds.
#endif

//#define DEBUG
#ifdef DEBUG
#define PrintDebug(x) { Serial.println(x); }
#else
#define PrintDebug(x);
#endif

const uint8_t SETTING_NUM = 9;
const short SETTING_RULE[SETTING_NUM][3] {
  {1, 1, 2},
  {10, 6, 13},
  {14, 7, 14},
  {13, 10, 30},
  {66, 25, 100},
  {203, 50, 250},
  { 10, 1, 100},
  { 15, 5, 60},
  { -1, 0, 0},
};

struct DATA_EEPROM{
  uint8_t BatteryType;
  uint8_t BatterySerie;
  uint8_t motorPoles;
  uint8_t motorPulley;
  uint8_t wheelPulley;
  uint8_t wheelSize;
  uint8_t Stepper;
  uint8_t SpeedLimit;
  uint64_t addr;
};

struct DATA_TX{
  uint8_t type;
  uint8_t BatteryType;
  uint8_t BatterySerie;
  uint8_t motorPoles;
  uint8_t motorPulley;
  uint8_t wheelPulley;
  uint8_t wheelSize;
  uint8_t Stepper;
  uint8_t SpeedLimit;
  uint64_t addr;
};

struct DATA_PACKET {
  uint8_t type;
  uint8_t mode;
  uint8_t trigger;
  uint16_t throttle;
};

struct DATA_VESC {
  uint8_t trigger;
  uint16_t throttle;
  float speed;
  float distance;
  int8_t tempFetFiltered;
  float inpVoltage;
  int8_t avgInputCurrent;
	int8_t ampHours;
	int8_t ampHoursCharged;
};

const uint8_t PIN_PPM = 0x05;
const uint16_t NEUTRAL = 1500;
const uint16_t P_MIN = 1000;
const uint16_t P_MAX = 2000;
const uint8_t SAFE_MARGIN = 25;
const uint8_t NEUTRAL_MARGIN = 150;

const uint64_t DEFAULT_ADDR PROGMEM = 0xE1E1E1E1E1;
const uint8_t LIMIT_TIMEOUT = 25;
const int8_t DEFAULT_CHANNEL PROGMEM = 0x71;

#endif