# RF Interceptor - 2.4GHz Packet Sniffer

This project turns an ESP32 microcontroller and two NRF24L01 radio modules into a portable 2.4GHz RF packet sniffer and interceptor. One module listens and captures packets across all 125 radio channels, and the second module can replay or re-transmit those captured packets. All captured data is streamed to your computer over USB serial or to a phone over Bluetooth.

This is a learning project. It is meant to help you understand how wireless communication works at the packet level.

---

## What This Device Does

- Scans all 125 channels in the 2.4GHz band (the same band used by WiFi, Bluetooth, and many wireless gadgets)
- Captures raw radio packets from NRF24-based devices nearby
- Logs each packet with its channel number and signal strength
- Can replay a captured packet using the second NRF module
- Sends all captured data to your browser or serial monitor in real time
- Runs on a single 3.7V LiPo battery for several hours

---

## Who This Is For

If you have never built an electronics project before, this guide will walk you through every single step. You do not need to know how to code. You just need to follow the wiring diagram carefully, install the right software, and upload the code. If something does not work, the Troubleshooting section at the bottom of this file covers the most common problems.

---

## Parts You Need

All parts are cheap and easy to find on Amazon, AliExpress, or your local electronics shop.

| Part | Quantity | Approximate Cost |
|---|---|---|
| ESP32 Development Board (30-pin, any brand) | 1 | $4 - $8 |
| NRF24L01 + PA + LNA Module (the one with the external antenna) | 2 | $2 - $4 each |
| 100uF capacitor (electrolytic, 10V or higher) | 2 | $0.50 |
| TP4056 LiPo Charger Module (with protection circuit) | 1 | $1 - $2 |
| 3.7V LiPo battery, 1000mAh or larger | 1 | $5 - $10 |
| Small toggle switch (SPDT or SPST) | 1 | $0.50 |
| Small project box or 3D printed enclosure | 1 | $2 - $5 |
| Jumper wires (male-to-female) | 20 | $2 |
| Breadboard or custom PCB | 1 | $2 |

Total estimated cost: $20 to $45 depending on where you buy.

Important note about the NRF24L01 modules: Buy the version labeled "NRF24L01 + PA + LNA with SMA antenna". This version has a power amplifier and a low-noise amplifier built in, which gives much better range. The tiny board-only version without the external antenna is less reliable.

---

## Wiring Diagram

The ESP32 uses one SPI bus shared between both NRF modules. The only difference between the two modules is which Chip Enable (CE) and Chip Select (CSN) pin each one uses. The SPI data lines (SCK, MOSI, MISO) are shared.

```
                    ESP32 Pin Layout
                    ----------------

  NRF MODULE 1 (Scanner)        NRF MODULE 2 (Transmitter)
  ----------------------        --------------------------
  VCC  --> 3.3V                 VCC  --> 3.3V
  GND  --> GND                  GND  --> GND
  CE   --> GPIO 4               CE   --> GPIO 16
  CSN  --> GPIO 5               CSN  --> GPIO 17
  SCK  --> GPIO 18              SCK  --> GPIO 18  (same pin)
  MOSI --> GPIO 23              MOSI --> GPIO 23  (same pin)
  MISO --> GPIO 19              MISO --> GPIO 19  (same pin)
  IRQ  --> not connected        IRQ  --> not connected

  BATTERY SYSTEM
  --------------
  LiPo (+) --> TP4056 BAT+
  LiPo (-) --> TP4056 BAT-
  TP4056 OUT+ --> Toggle Switch --> ESP32 VIN
  TP4056 OUT- --> ESP32 GND
  USB port on TP4056 is used to recharge the battery
```

Place a 100uF capacitor between the VCC and GND pins of each NRF module. Put the capacitor as close to the NRF module pins as possible. This is very important. Without these capacitors, the NRF modules will reset randomly or not work at all because they draw large current spikes when transmitting.

---

## Software Setup

### Step 1 - Install Arduino IDE

Go to https://www.arduino.cc/en/software and download the Arduino IDE for your operating system. Install it like any normal program.

### Step 2 - Add ESP32 Board Support

1. Open Arduino IDE
2. Go to File > Preferences
3. In the box that says "Additional boards manager URLs", paste this link:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
4. Click OK
5. Go to Tools > Board > Boards Manager
6. Search for "esp32" and install the package by Espressif Systems
7. Wait for it to finish downloading (it may take a few minutes)

### Step 3 - Install the RF24 Library

1. In Arduino IDE, go to Sketch > Include Library > Manage Libraries
2. Search for "RF24"
3. Install the library by TMRh20 (make sure it is by TMRh20, not another author)
4. Also search for "RF24Network" and install that too

### Step 4 - Select Your Board

1. Go to Tools > Board > esp32 > ESP32 Dev Module
2. Go to Tools > Port and select the COM port your ESP32 is connected to
   (on Windows it will be something like COM3 or COM4, on Mac it will be /dev/cu.usbserial-something)

### Step 5 - Upload the Code

1. Open the file `src/main.cpp` from this project
2. If you are using the Arduino IDE (not PlatformIO), rename it to `rf_interceptor.ino` and open that
3. Click the Upload button (the right arrow icon)
4. Wait for it to say "Done uploading"

---

## How to Use It

### Basic Operation

1. Turn on the device using the toggle switch
2. Open Arduino IDE and go to Tools > Serial Monitor
3. Set the baud rate to 115200
4. You will start seeing channel scan results immediately

### Web Interface

Open the file `demo/index.html` in your browser. This file connects to the ESP32 over Web Serial (Chrome and Edge only) and shows you a live channel activity map. You can see which channels have traffic and view the raw packet bytes.

### LED Status Indicators

If you add an LED to GPIO 2 (the built-in LED on most ESP32 boards), it will blink once when a packet is captured and stay solid when the device is scanning and idle.

---

## Project File Structure

```
rf-interceptor/
├── README.md              <- You are reading this
├── TROUBLESHOOTING.md     <- Fix common problems here
├── HARDWARE.md            <- Detailed hardware notes
├── platformio.ini         <- PlatformIO build config (optional)
├── .gitignore             <- Files Git should ignore
├── src/
│   ├── main.cpp           <- Main program code
│   ├── config.h           <- Settings you can change
│   ├── nrf_handler.h      <- NRF module header
│   ├── nrf_handler.cpp    <- NRF module logic
│   ├── ble_server.h       <- Bluetooth server header
│   └── ble_server.cpp     <- Bluetooth server logic
├── hardware/
│   ├── wiring.md          <- Step by step wiring guide
│   └── parts_list.md      <- Full parts list with links
├── demo/
│   └── index.html         <- Browser-based live monitor
└── docs/
    └── how_it_works.md    <- Theory of operation
```

---

## Troubleshooting

See the full `TROUBLESHOOTING.md` file for detailed help. Here are the most common problems:

**NRF module not detected**
Almost always caused by missing capacitors or wrong wiring. Double-check every wire against the diagram above.

**Garbled output in serial monitor**
Make sure your serial monitor baud rate is set to 115200.

**No packets captured**
Try moving the device close to a known NRF device. The scanner cycles through all channels so it may take a few seconds to reach the right one.

**ESP32 keeps crashing or rebooting**
This usually means the battery or USB port cannot supply enough current. The NRF PA+LNA modules draw up to 115mA each. Make sure your power supply can handle at least 500mA total.

---

## Legal Notice

This tool is for educational purposes and authorized testing only. Intercepting wireless communications you do not own or have permission to test may be illegal in your country. Always get permission before using this device on any network or device you do not own.

---

## License

MIT License. See LICENSE file for details.
