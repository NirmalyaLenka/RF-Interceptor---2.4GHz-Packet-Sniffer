# Parts List

## Required Components

| Component | Spec | Why You Need It |
|---|---|---|
| ESP32 Dev Board | 30-pin, dual-core 240MHz | The brain of the device. Runs the code, manages the radio modules, and provides Bluetooth to your phone. |
| NRF24L01 + PA + LNA | With SMA external antenna, 2.4GHz | Radio module 1 acts as the scanner. Radio module 2 acts as the transmitter/replayer. Buy 2. |
| 100uF Electrolytic Capacitor | 10V or 16V, THT | One per NRF module. Prevents voltage droop when the radio transmits. Without these, the modules will misbehave. |
| TP4056 Charger Module | With protection circuit (DW01 chip) | Safely charges the LiPo battery over USB and cuts power if the battery gets too low. |
| 3.7V LiPo Battery | 1000mAh minimum, JST-PH connector | Powers the device when not plugged into USB. Bigger capacity = longer runtime. |
| SPDT Toggle Switch | Any small panel-mount toggle | Main on/off switch. Breaks the power line between TP4056 output and ESP32 VIN. |
| Jumper Wires | Male-to-female, 10cm or 15cm | Connects the modules together. Get at least 20 to have spares. |
| Breadboard | Half-size (400 point) or full | Holds everything together during prototyping without soldering. |
| Project Box | 100mm x 60mm x 30mm or larger | Keeps everything safe and makes it portable. |

## Optional but Recommended

| Component | Purpose |
|---|---|
| 10uF ceramic capacitor (2x) | Place in parallel with the 100uF electrolytic for better high-frequency filtering |
| LED + 220 ohm resistor | External status LED if your ESP32 board does not have a built-in LED on GPIO 2 |
| Right-angle SMA to SMA extension | Routes the NRF module antennas outside the enclosure for better signal |
| JST-PH 2.0 connector pair | Allows you to easily disconnect the battery from the TP4056 |
| Pin headers (2.54mm pitch) | If you want to solder modules to a perfboard instead of using jumper wires |

## Where to Buy

All of these components are available from:
- AliExpress (cheapest, ships slowly from China, usually 2-4 weeks)
- Amazon (faster, slightly more expensive, look for fulfilled-by-Amazon listings)
- Adafruit or SparkFun (most reliable quality, best documentation, US-based)
- LCSC or DigiKey (for the capacitors and discrete components)

Search terms that work well:
- "ESP32 DevKit 30 pin" or "ESP32 DOIT DevKit V1"
- "NRF24L01 PA LNA SMA antenna module"
- "TP4056 lithium charger protection module"
- "3.7V lipo battery 1000mah JST"
