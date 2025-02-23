#include "Receiver.h"

RECEIVER receiver;

/* Implement WDT - Watch-Dog Timer
  <avr/iotn202.h>
  WDT_PERIOD_t
*/
void resetFunc(){
  /* 8 cycles (8ms) */
  wdt_enable(0x01<<0);
  while(true);
}

void setup(){
  receiver.begin();
}

void loop(){
  if(!receiver.NRF.checkNRF24()){
    if(receiver.VESC.throttle!=NEUTRAL){
      receiver.VESC.throttle = NEUTRAL;
    }else{
      #ifndef DEBUG
      resetFunc();
      #endif
    }
  }else{
    receiver.NRF.readPacket();
    receiver.checkTrigger();
    receiver.checkThrottle();
    #ifdef LED_INDICATOR
    receiver.LedIndicator();
    #endif

    if(receiver.NRF.Px.type==1){
      #ifndef DEBUG
      receiver.checkVESC();
      #endif
      receiver.commit();
    }
  }
}