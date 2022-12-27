# Smart DSMR P1 Splitter

Device that splits the P1 port into two individual operating P1 ports (P2 and P3).
It let's two devices communicate with the DSMR P1 port at different intervals.
In my case I use this device to connect a HomeWizard P1 meter and the load balancer of an Alfen EV Charger.

## How it works

## Schema
![SCH](/media/Schematic_Smart_DSMR_Splitter_2022-12-27.png?raw=true "SCH")

## PCB
![PCB](/media/PCB_impression.png?raw=true "PCB")

![PCB](/media/PCB-top.png?raw=true "PCB")

![PCB](/media/PCB-bottom.png?raw=true "PCB")

## Todo
+ Add a display and show some relevant info
+ Add MQTT functionality
+ Add Home Assistant integration

## Libraries Used

+ [Arduino Dutch Smart meter (DSMR) parser](https://github.com/matthijskooijman/arduino-dsmr) (slightly modified)

