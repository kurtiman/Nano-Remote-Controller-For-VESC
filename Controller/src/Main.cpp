#include "Controller.h"

CONTROLLER controller;

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
    controller.begin();
    if (controller.NRF.Px.trigger==1){
        controller.NRF.Px.type = 2;
    }else{
        controller.NRF.Px.type = 1;
    }
}
void loop(){
    if(!controller.NRF.checkNRF24()){
        resetFunc();
    }else{
        controller.readBattery();
        controller.readHallSensor();

        switch (controller.NRF.Px.type){
            case 1:
                controller.controlMain();
            break;
            case 2:
                controller.controlSetting();
            break;
        }
    }
    controller.NRF.send();
    controller.OLED.update(50);
}