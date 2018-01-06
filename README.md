# TheftSystem
### About
- Theft system based on Arduino Uno R3 with different types of sensors
### Hardware
- Door sensor : IR Sensor ( white band )
```
VCC -> 5V
GND -> GND
D0  -> PIN2
```
- Lock sensor : RFID
```
VCC  -> 3.3V
RST  -> PIN9
GND  -> GND
IRQ  -> NOT
MISO -> PIN12
MOSI -> PIN11
SCK  -> PIN13
SDA  -> PIN10
```
- Notify sensor : Buzzer + LEDs
```
BUZZER -> PIN3
GND    -> GND
GREEN  -> PIN4
RED    -> PIN5
```
- GSM Shield : SIM900
```
PIN7 -> PIN7
PIN8 -> PIN8
PIN9 -> PIN6
GND  -> GND

SOURCE POWER : 5V 1.5A
```
### Features
- Posibility for unlock / lock with multiple RDID keys
- Call multiple phone numbers when an incident appear
