# SalahTime
Device that shows prayer time and Qibla direction anywhere & anytime without any internet connection
<img width="2298" height="3498" alt="Zine" src="https://github.com/user-attachments/assets/e221e65e-4aac-461b-958f-1c764192fc8f" />

## What is it?
It is an ESP32-powered device with buttons, GNSS receiver, RTC, magnetic sensor, and E-Paper display. By using all of these, it is planned to show prayer time using latitude and longitude alongside with direction to Qibla by magnetic sensor. Because there is an E-Paper display, battery can last for many-many days. If the battery will be drained, it is easy to charge using universal Type-C port. Currently, it shows prayer time based on location, but will have full interface and direction to Qibla in the future.

## Why did I build it?
I was walking in Shenzhen after Fallout with my phone died and then realized that I have to read Salah
I could not know if the next salah already started and I needed the direction to Qibla
Strangers there do not understand English, so I could not communicate with them
Then, I got an idea of internet-free device that can predict prayer time and show the direction to Qibla

## Steps to Reproduce
1. Order PCB and parts from any provider(Example: JLCPCB)
2. Assemble and solder parts to PCB
3. 3D Print CAD models from "CAD" folder
4. Push software into MCU and download required libraries using Arduino IDE 
5. Use!

## Arduino IDE Libraries:
GxEPD2
TinyGPS++
RTClib

## Bill of Materials
| Name | Quantity | Price |
| --- | --- | --- |
| DS3231M RTC | 1 | 3.25$ |
| ATGM336H GNSS | 1 | 2.22$ |
| ESP32-C3FH4 | 1 | 2.18$ |
| Vibration Motor | 1 | 0.56$ |
| Tactile Push Buttons | 5 | 0.3$ |
| Ceramic Active Antenna | 1 | 0.73$ |
| Coin Cell Holder | 1 | 1.27$ |
| MAX17048 fuel gauges | 1 | 2.18$ |
| ME6211 LDO to 3.3v | 1 | 0.52$ |
| NPN Transistor | 1 | 0.02$ |
| 2.9-inch E Ink Display | 1 | 4$ |
| 803040 1000 mAh LiPo | 1 | 4$ |
| Type-C 8 Pin | 1 | 0.04$ |
| 2.5mm JST Connector | 1 | 0.04$ |
| SI1308EDL N-MOSFET | 1 | 0.1$ |
| TP4056 | 1 | 0.2$ |
| FS8205A | 1 | 0.05$ |
| DW01A | 1 | 0.5$ |
| E Ink connector | 1 | 0.11$ |
| QST QMC5883P | 1 | 1.6$ | 
| 0,1uF C | 6 | 0.2$ |
| 10uF | 3 | 0.2$ | 
| 1uF/25V | 14 | 0.7$ |
| 4.7uF/25V | 3 | 0.2$ |
| 15pF | 2 | 0.03$ |
| 10nF | 1 | 0.02$ |
| THT 3mm diodes | 2 | 0.07$ |
| MBR0530 Diode | 3 | 0.02$ |
| 47nH L | 1 | 0,06$ |
| 47uH L | 1 | 0.08$ |
| 2nH L | 1 | 0.01$ |
| 10K R | 1 | 0.01$ |
| 1K R | 4 | 0.01$ |
| 1.2K R | 1 | 0.05$ |
| 4.7K R | 2 | 0.07$ |
| 100 R | 1 | 0.01$ |
| 1M R | 1 | 0.01$ |
| 2.2 R | 1 | 0.01$ |
| 499 R | 1 | 0.01$ |
| 0 R | 1 | 0.01$ |
| 10pF Crystal | 1 | 0.3$ | 
| TOTAL: | 26$ |

## Images
<img width="1920" height="1080" alt="salah_tracker" src="https://github.com/user-attachments/assets/48cdc31d-170c-485c-80e0-01c66805ff1f" />
<img width="1920" height="1080" alt="salah_tracker2" src="https://github.com/user-attachments/assets/b2ca6a53-ccb7-42b1-838c-6b09cbc7a980" />
[schematics.pdf](https://github.com/user-attachments/files/32201017/schematics.pdf)
<img width="1230" height="831" alt="PCB_Front" src="https://github.com/user-attachments/assets/c521ee1a-41ee-4769-b55d-5fb4604b6de5" />
<img width="1260" height="926" alt="PCB_Behind" src="https://github.com/user-attachments/assets/bc01117f-ce0a-4337-bb0a-a14fcc366712" />

