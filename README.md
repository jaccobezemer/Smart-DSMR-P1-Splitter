# Smart DSMR P1 Splitter

Device that splits the P1 port into two individually functioning P1 ports (P2 and P3).
It will let two devices communicate with the DSMR P1 port at different intervals.
In my case I use this device to connect a HomeWizard P1 meter and the load balancer of an Alfen EV Charger.

## How it works
A level shifter and inverter (Q1, BS170) is attached to UART1 (P1) of an ESP32 (U1). The ESP32 processes the serial stream using the DSMR parser library and stores a complete telegram in memory.
Devices are attached via optocoupler (U4, VO2631) with the other two UART's (UART2 = P2, UART0 = P3) and are able to request the stored telgram at their own interval (typically 1 or 10 seconds) by bringing the data request signal high. The data request signals are attached with the ESP32 by optocoupler U3 (LTV-827S). The specs for P1, P2 and P3 are conform to the DSMR specification. 

## Schematics
![Schema](/media/Schematic_Smart_DSMR_Splitter_2022-12-27.png?raw=true "Schema")

## PCB
![PCB top](/media/PCB-top.png?raw=true "PCB top")

![PCB bottem](/media/PCB-bottom.png?raw=true "PCB bottom")

## Todo
+ Add a display and show some relevant info
+ Add MQTT functionality
+ Add Home Assistant integration

## Libraries Used

+ [Arduino Dutch Smart meter (DSMR) parser](https://github.com/matthijskooijman/arduino-dsmr) (slightly modified)

