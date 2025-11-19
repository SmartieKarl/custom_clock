# **PRIVATE CODE, REMOVE SENSITIVE INFO FROM CONFIG.H BEFORE MAKING PUBLIC**

# Alarm Clock Project

This is an ESP32 based alarm clock with RFID functionality. The idea behind
this is that in order to turn off your alarm in the morning, you have to
physically get out of bed, grab your NFC card, and tap it on the reciever to
turn your alarm off. At that point you'll be more awake and have less of a
reason to go back to bed.

--------------------------------------------------------------------------

**© 2025 Michael Marsh. All rights reserved.**

I worked really hard on this project. Unauthorized copying or commercial use is prohibited. Thanks!

**[See COPYRIGHT file for complete terms and conditions.](./COPYRIGHT)**

--------------------------------------------------------------------------





# Setup

## Hardware

- **Microcontroller:** ESP-WROOM-32 Dev Module
- **Display:** ST7789 240×320 SPI TFT
- **RTC:** DS3231 (I²C)
- **Audio:** DFPlayer Mini (UART)
- **NFC Functions:** RFID-RC522 (SPI) for NFC tags

## Pin Connections

### ST7789 Display
| TFT Pin | ESP32 Pin |
|---------|-----------|
| CS      | GPIO15    |
| DC      | GPIO2     |
| RST     | GPIO4     |
| MOSI    | GPIO23    |
| SCLK    | GPIO18    |
| LED     | GPIO26    |

### DS3231 RTC (I²C)
| RTC Pin | ESP32 Pin |
|---------|-----------|
| SDA     | GPIO21    |
| SCL     | GPIO22    |

### DFPlayer Mini (UART)
| DFPlayer | ESP32 Pin |
|----------|-----------|
| TX       | GPIO16    |
| RX       | GPIO17    |

### RFID-RC522 (SPI)
| RFID Pin | ESP32 Pin |
|----------|-----------|
| SDA/SS   | GPIO5     |
| SCK      | GPIO18    |
| MOSI     | GPIO23    |
| MISO     | GPIO19    |
| GND      | GND       |
| RST      | GPIO25    |
| 3.3V     | 3.3V      |


## Other wiring

**DFPlayer mini voltage divider**
- So the DFPlayer's TX pin doesn't send 5v signals back to GPIO16 (bad)
- TX -> 1kΩ resistor -> GPIO16 -> 2.2kΩ resistor -> GND

**Photoresistor Circuit:**
- For auto-brightness functions
- 3.3v -> photoresistor -> GPIO34 -> 4.7kΩ resistor -> GND



### Power Supply
- **ESP32 Module:** 5v DC (regulated to 3.3v on-board)
- **TFT Display:** 3.3V (from ESP32)
- **DS3231 RTC:** 3.3V  (from ESP32)
- **DFPlayer Mini:** 5V (from ESP32)
- **RFID-RC522:** 3.3v (from ESP32)


## Software
Software is designed to work with **PlatformIO IDE** on **VSCode**.
Make sure they're installed for this to work correctly!