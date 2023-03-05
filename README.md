# Smart DSMR P1 Splitter

Device that splits the P1 port into two individually functioning P1 ports (P2 and P3).
It will let two devices communicate with the DSMR P1 port at different intervals.
In my case I use this device to connect a HomeWizard P1 meter and the load balancer of an Alfen EV Charger.

## How it works
A level shifter and inverter (Q1, BC848B) is attached to UART1 (P1) of an ESP32 (U1). The ESP32 processes the serial stream using the DSMR parser library and stores a complete telegram in memory.
Devices are attached via optocoupler (U4, HCPL-0631) with the other two UART's (UART2 = P2, UART0 = P3) and are able to request the stored telgram at their own interval (typically 1 or 10 seconds) by bringing the data request signal high. The data request signals are attached with the ESP32 by optocoupler U3 (HCPL-0631). The specs for P1, P2 and P3 are conform the DSMR specification.

## Schematics
![Schema](/media/Schematic_Smart_DSMR_Splitter_v1.2.png?raw=true "Schema")

## Several PCB images
![Prototype](/media/prototype.jpg?raw=true "Prototype")

![Working prototype](/media/working-prototype.jpg?raw=true "Working prototype")

![PCB top](/media/PCB_top_v1.2.png?raw=true "PCB top")

![PCB bottom](/media/PCB_bottom_v1.2.png?raw=true "PCB bottom")

![PCB top placed](/media/PCB_top_placed_v1.2.png?raw=true "PCB top placed")

![PCB bottom placed](/media/PCB_bottom_placed_v1.2.png?raw=true "PCB bottom placed")

## Todo
+ Add a display and show some relevant info
+ Add MQTT functionality
+ Add Home Assistant integration

## Libraries Used

+ [Arduino Dutch Smart meter (DSMR) parser](https://github.com/matthijskooijman/arduino-dsmr) (slightly modified)

License
-------
All of the code in this project is licensed under the GNU General Public License v3.0 (GNU GPLv3) license.