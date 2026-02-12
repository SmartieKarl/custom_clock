# Michael's Custom Clock

This is an ESP32 based alarm clock with RFID functionality. The idea behind
this is that in order to turn off your alarm in the morning, you have to
physically get out of bed, grab your NFC card, and tap it on the reciever to
turn your alarm off. At that point you'll be more awake and have less of a
reason to go back to bed.

--------------------------------------------------------------------------

**[Please see LICENSE file for complete terms and conditions.](./LICENSE)**

--------------------------------------------------------------------------

# Setup

## Hardware

### Boards
- **Microcontroller:** ESP-WROOM-32 Dev Module ([Amazon](https://www.amazon.com/YEJMKJ-ESP-WROOM-32-Development-Dual-Mode-Microcontroller/dp/B0CDRKX814/ref=asc_df_B0CDRKX814?tag=bingshoppinga-20&linkCode=df0&hvadid=80814296089924&hvnetw=o&hvqmt=e&hvbmt=be&hvdev=c&hvlocint=&hvlocphy=95933&hvtargid=pla-4584413766458666&msclkid=eca191cbe8a91cf55b29f845eb49b1a4&th=1)) ([AliExpress](https://www.aliexpress.us/item/3256809488479021.html?spm=a2g0o.productlist.main.24.2dbfyPnlyPnlQK&algo_pvid=44c7fdf2-0aac-4c17-8439-820ed06d7e9f&algo_exp_id=44c7fdf2-0aac-4c17-8439-820ed06d7e9f-21&pdp_ext_f=%7B%22order%22%3A%22107%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21USD%2114.36%210.99%21%21%2198.59%216.81%21%402103212517709150759176839e3c1f%2112000049830321688%21sea%21US%210%21ABX%211%210%21n_tag%3A-29910%3Bd%3Af98f678%3Bm03_new_user%3A-29895%3BpisId%3A5000000197847450&curPageLogUid=jRsQ3vw5LUS7&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005009674793773%7C_p_origin_prod%3A))
- **Display:** ST7789 240×320 SPI TFT ([Amazon](https://www.amazon.com/XIITIA-Display-Screen-240%C3%97320-Arduino/dp/B0D5HCDNY5/ref=asc_df_B0D5HCDNY5?tag=bingshoppinga-20&linkCode=df0&hvadid=80745586183882&hvnetw=o&hvqmt=e&hvbmt=be&hvdev=c&hvlocint=&hvlocphy=95933&hvtargid=pla-4584345054934476&psc=1&msclkid=8413e1b7868a1f540a83883983540697)) ([AliExpress](https://www.aliexpress.us/item/3256806903354055.html?spm=a2g0o.productlist.main.2.6c96mxWfmxWfwi&algo_pvid=186499c0-2536-4aa5-acff-9441a945c34d&algo_exp_id=186499c0-2536-4aa5-acff-9441a945c34d-1&pdp_ext_f=%7B%22order%22%3A%22249%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21USD%214.15%210.99%21%21%214.15%210.99%21%40210328d417709154299382545ee695%2112000051013549131%21sea%21US%210%21ABX%211%210%21n_tag%3A-29910%3Bd%3Af98f678%3Bm03_new_user%3A-29895%3BpisId%3A5000000197847450&curPageLogUid=Em2ocZTVrThQ&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005007089668807%7C_p_origin_prod%3A))
- **RTC:** DS3231 ([Amazon](https://www.amazon.com/CANADUINO%C2%AE-DS3231-Module-Interface-Battery/dp/B07BCPRH6F/ref=asc_df_B07BCPRH6F?tag=bingshoppinga-20&linkCode=df0&hvadid=80401998416312&hvnetw=o&hvqmt=e&hvbmt=be&hvdev=c&hvlocint=43847&hvlocphy=&hvtargid=pla-4584001471079757&psc=1&msclkid=83740f833a4d1fc93371660d3d29bd78)) ([AliExpress](https://www.aliexpress.us/item/3256806957282138.html?spm=a2g0o.productlist.main.3.771b68a7Vw6zQf&algo_pvid=e4d7548a-b13a-4fe3-8c66-4fdd6110ec87&algo_exp_id=e4d7548a-b13a-4fe3-8c66-4fdd6110ec87-2&pdp_ext_f=%7B%22order%22%3A%227442%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21USD%211.85%210.99%21%21%2112.68%216.78%21%40210319b717709158313054733ec96d%2112000039565918038%21sea%21US%210%21ABX%211%210%21n_tag%3A-29910%3Bd%3Af98f678%3Bm03_new_user%3A-29895%3BpisId%3A5000000197847491&curPageLogUid=V5v2XZiEjaeZ&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005007143596890%7C_p_origin_prod%3A))
- **Audio:** DFPlayer Mini ([Amazon](https://www.amazon.com/DFPlayer-A-Mini-MP3-Player/dp/B089D5NLW1/ref=asc_df_B089D5NLW1?tag=bingshoppinga-20&linkCode=df0&hvadid=80127099786506&hvnetw=o&hvqmt=e&hvbmt=be&hvdev=c&hvlocint=&hvlocphy=95553&hvtargid=pla-4583726563169858&psc=1)) ([AliExpress](https://www.aliexpress.us/item/3256808368233947.html?spm=a2g0o.productlist.main.13.3bf76837mFTYeM&utparam-url=scene%3Asearch%7Cquery_from%3Apc_back_same_best%7Cx_object_id%3A1005008554548699%7C_p_origin_prod%3A1005008627424015&algo_pvid=9bea87f7-b73c-430e-bb4d-43f9eaea313f&algo_exp_id=9bea87f7-b73c-430e-bb4d-43f9eaea313f&pdp_ext_f=%7B%22order%22%3A%2211470%22%2C%22spu_best_type%22%3A%22order%22%2C%22orig_sl_item_id%22%3A%221005008554548699%22%2C%22orig_item_id%22%3A%221005008627424015%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21USD%213.03%211.33%21%21%2120.82%219.16%21%402101e62517709159912257220ecbe6%2112000045690041635%21sea%21US%210%21ABX%211%210%21n_tag%3A-29910%3Bd%3Af98f678%3Bm03_new_user%3A-29895&gatewayAdapt=4itemAdapt))
- **NFC Functions:** RFID-RC522 ([Amazon](https://www.amazon.com/HiLetgo-RFID-Kit-Arduino-Raspberry/dp/B01CSTW0IA/ref=sr_1_7?crid=KJONX0IYNQD1&dib=eyJ2IjoiMSJ9.ZBUvY56h1-nswPF8JiKQsqBwxPzBYf7-6SpCUyEn8OKrM2IzwMuBt9t-LKKVV0weiyRENzppSduIBTk7EKQLMxWlD7ug5VewFG8vSS6TL6JisdyXUI2wg1-h9PAxyuZzN7LSXrliIkK20Iyek9shkE3fXwI87OVCbY2rfR0yjmVduRzqpHOdDBdkIPrn_aPrJyYCpRw-boJmC9xi-NH1wD4m8MWyMC3rNLkjCSYopZ9yR3kjgxeCFRY-H8hLWgAwQSy2r7JA5_rkQ_Ml_gy6K4JHqUt04a4d9Ts3ZEO4pmI.b5fmKgqIbH9pf65JVM5GETF5uedXYhVBHl_dAH3MRB8&dib_tag=se&keywords=rfid-rc522&qid=1770916075&s=electronics&sprefix=rfid-rc522%2Celectronics%2C229&sr=1-7&th=1)) ([AliExpress](https://www.aliexpress.us/item/3256806642614131.html?spm=a2g0o.productlist.main.14.3a21584fYpkNGt&utparam-url=scene%3Asearch%7Cquery_from%3Apc_back_same_best%7Cx_object_id%3A1005006828928883%7C_p_origin_prod%3A&algo_pvid=fb770c16-c86f-486f-8104-455a05232710&algo_exp_id=fb770c16-c86f-486f-8104-455a05232710&pdp_ext_f=%7B%22order%22%3A%22253%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21USD%211.84%210.99%21%21%2112.64%216.81%21%402101e8f317709162070488858e856f%2112000038437495022%21sea%21US%210%21ABX%211%210%21n_tag%3A-29910%3Bd%3Af98f678%3Bm03_new_user%3A-29895%3BpisId%3A5000000197847418&gatewayAdapt=4itemAdapt))

### Misc hardware
- a 2" speaker with two jumper wires. I used [this one](https://www.aliexpress.us/item/3256809273226082.html?spm=a2g0o.detail.pcDetailTopMoreOtherSeller.1.5afbTihETihER0&gps-id=pcDetailTopMoreOtherSeller&scm=1007.40050.354490.0&scm_id=1007.40050.354490.0&scm-url=1007.40050.354490.0&pvid=ea5149e1-a9d5-4bd2-9c6d-d897df53609c&_t=gps-id:pcDetailTopMoreOtherSeller,scm-url:1007.40050.354490.0,pvid:ea5149e1-a9d5-4bd2-9c6d-d897df53609c,tpp_buckets:668%232846%238109%231935&pdp_ext_f=%7B%22order%22%3A%22272%22%2C%22eval%22%3A%221%22%2C%22sceneId%22%3A%2230050%22%2C%22fromPage%22%3A%22recommend%22%7D&pdp_npi=6%40dis%21USD%2111.22%210.99%21%21%2177.01%216.78%21%402101df0e17709188654595016e0e1b%2112000049170771831%21rec%21US%21%21ABXZ%211%210%21n_tag%3A-29910%3Bd%3Af98f678%3Bm03_new_user%3A-29895%3BpisId%3A5000000197847434&utparam-url=scene%3ApcDetailTopMoreOtherSeller%7Cquery_from%3A%7Cx_object_id%3A1005009459540834%7C_p_origin_prod%3A), they come in sets of 2.
- 4 tactile buttons (like [these]())
- a 8-pin header (Optional, but recommended for RFID scanner range to be adequate) [here](https://www.aliexpress.us/item/3256803424019097.html?spm=a2g0o.productlist.main.1.7732eOVqeOVqkZ&algo_pvid=f7f01241-a198-4716-9f68-11508b0911d9&algo_exp_id=f7f01241-a198-4716-9f68-11508b0911d9-0&pdp_ext_f=%7B%22order%22%3A%224905%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21USD%213.86%210.99%21%21%213.86%210.99%21%402101c44517709168716831516edaa4%2112000026601252522%21sea%21US%210%21ABX%211%210%21n_tag%3A-29910%3Bd%3Af98f678%3Bm03_new_user%3A-29895%3BpisId%3A5000000198352206&curPageLogUid=5fMaau2sdKL1&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005003610333849%7C_p_origin_prod%3A)
- 14-pin ribbon cable [here](https://www.aliexpress.us/item/3256805224133202.html?spm=a2g0o.productlist.main.17.70c9VXaMVXaMPb&algo_pvid=d7e3322f-eccb-495a-9034-0816200b9593&algo_exp_id=d7e3322f-eccb-495a-9034-0816200b9593-16&pdp_ext_f=%7B%22order%22%3A%2215%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&pdp_npi=6%40dis%21USD%217.93%210.99%21%21%217.93%210.99%21%402103129017709170436946618e5790%2112000032947330320%21sea%21US%210%21ABX%211%210%21n_tag%3A-29910%3Bd%3Af98f678%3Bm03_new_user%3A-29895%3BpisId%3A5000000197847434&curPageLogUid=mJgWqnB3B9uD&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005005410447954%7C_p_origin_prod%3A) 
- 2 2x7 ribbon cable through hole connectors [here]()
- a 2x7 header [here]()
- a microsd card for DFPlayer (max 32gb)
- a photoresistor
- a 1kΩ resistor, 2.2kΩ resistor, and 4.7kΩ resistor
- a 10μF capacitor and a 0.1μF capacitor
- a usb-c cable (only if you want use electricity with it)

**Make sure all hardware you get matches the hardware on the boards in the photos at the bottom of this file**

## Pin Connections

### ST7789 Display
| TFT Pin | ESP32 Pin |
|---------|-----------|
| CS      | GPIO15    |
| RST     | GPIO4     |
| DC      | GPIO2     |
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

**Buttons**
There are four simple tactile buttons in use, they all use ESP32's
internal pull-up resistors:
- Button 1 (screen top left): GPIO14 -> button1 -> GND 
- Button 2 (screen top right): GPIO27 -> button2 -> GND
- Button 3 (screen bottom left): GPIO33 -> button3 -> GND
- Button 4 (screen bottom right): GPIO32 -> button4 -> GND

**DFPlayer mini voltage divider**
- So the DFPlayer's TX pin doesn't send 5v signals back to GPIO16 (bad)
- TX -> 1kΩ resistor --> GPIO16
                      |
                      -> 2.2kΩ resistor -> GND

**Photoresistor Circuit:**
- For auto-brightness functions
- 3.3v -> photoresistor -> GPIO34
                        |
                        -> 4.7kΩ resistor -> GND

### Power Supply
- **ESP32 Module:** 5v DC (regulated to 3.3v on-board)
- **TFT Display:** 3.3V (from ESP32)
- **DS3231 RTC:** 3.3V  (from ESP32)
- **DFPlayer Mini:** 5V (from ESP32)
- **RFID-RC522:** 3.3v (from ESP32)


## Software
Software is designed to work with **PlatformIO IDE** on **VSCode**.
Make sure they're installed for this to work correctly!

# Assembly

## Materials
You will need:
- All listed hardware above
- Access to a 3d printer and PCB mill (there are online services)
- Soldering iron
- 2 <1 inch 5mm screws
- 2 matching bolts for screws

## Printing & Milling
- Included in this repo is a [clock_parts](./clock_parts) folder. It contains the
    - gerber files (Already formatted for PCB milling)
    - stl files (for 3d printing of the clock shell)

- 3d print both shell pieces and 2 copies of the board spacer piece. The display piece **must** be printed clear for the
photoresistor to do it's thing.

- Mill the boards (must be electroplated and double-sided).

- Solder everything to boards (like pictures below)

- Place bolts in the shell (in the suspiciously nut-shaped divots) You might need to use pliers to press them in there.
If you're desperate, you can get them hot with a soldering iron to melt them in.

- Slide main board into the main shell (so the usb port is lined up with the back)

- Push display board into the front display piece, lining up the photoresistor with the rectangle cut out on the inside.

- plug the speaker into the main board, and the display board into the main board.

- Close up the shell and screw the screws in.

- Assembled!