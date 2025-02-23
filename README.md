# Nano Remote Controller For VESC
This is a compact remote control with 128x32 telemetry rear-mounted display for VESC devices compatible for both regular and goofy

cloned from:  
------------------------------------------------------------------  
https://gitlab.com/deakbannok/Controller  

This project is deployed with Visual Studio Code with PlatformIO IDE extension.  
You may use other IDE to initialize the code as you prefer.  
For the receiver module:  
https://gitlab.com/deakbannok/receiver  
Receiver module requires a custom library dependency to compile.  
Please download and copy to your library folder.  
https://gitlab.com/deakbannok/receiver/-/tree/main/lib/VescUartControl  
For more instruction:  
https://gitlab.com/deakbannok/Controller/wikis/  
------------------------------------------------------------------  
https://gitlab.com/deakbannok/receiver  

Schematic:
https://gitlab.com/deakbannok/receiver/-/wikis/  
------------------------------------------------------------------  
https://www.thingiverse.com/thing:3322400  

Summary
UPDATED 01/26/2023

Upload a new case for PCB version.
Re-enforce spring load pole from to 5mm.
Implementation 01/01/2023

PCB version.
LED indicator on receiver.
Lost connection 2 stages safety.
Cruise Controller mode.
Speed Limit mode.
Requires (PPM+UART) on VESC enable.

https://gitlab.com/deakbannok/controller/
This is a compact remote control with 128x32 telemetry rear-mounted display for VESC devices compatible for both regular and goofy.

  
It uses both PPM (analog) and UART as control mode for tuning the throttle curve and ramping time.

The remote can add up to 3+ multiple profiles to allow switching between different VESC devices. With a custom VESC library dependency, it runs VESC HW 4.10+ (Tested with HW4.10 and FSESC 6.6 Dual).

The firmware is fully customizable with additional safety function.


UPDATED 11/01/2020

Rework BOTTOM (removed walls and expanded space)
Rework TOP (Added counterbore for M2 screws)
UPDATED 01/10/2020

THROTTLE (adjust traverse distance between magnetic)
reduce deadband.
UPDATED 01/10/2020

Rear mounted OLED
-Fits Right and Left hand
-Doesn't interfere with NRF24 transmission
Custom Section

