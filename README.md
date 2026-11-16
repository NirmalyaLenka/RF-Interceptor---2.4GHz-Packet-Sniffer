# RF Interceptor - 2.4GHz Packet Sniffer

This project turns an ESP32 microcontroller and two NRF24L01 radio modules into a portable 2.4GHz RF packet sniffer and interceptor. One module listens and captures packets across all 125 radio channels, and the second module can replay or re-transmit those captured packets. All captured data is streamed to your computer over USB serial or to a phone over Bluetooth.

This is a learning project. It is meant to help you understand how wireless communication works at the packet level.( illegal use can warm your butt)

---

## What this prototype actually does

This is an experimental ESP32 project using two NRF24L01 radios. It cycles through NRF24 channel numbers 0–124, reports carrier activity, and can receive packets that match the radio address configured in the firmware. Captured results are printed to Serial Monitor and can be sent to a connected BLE client.

Important limits:

- This is **not** a general-purpose 2.4 GHz, Wi-Fi, or Bluetooth packet sniffer. NRF24 hardware filters packets by address; the code's carrier-detect result is not the same as decoding a packet.
- The firmware includes a replay capability for controlled lab experiments. Use it only with devices you own or have explicit permission to test. Do not target locks, alarms, vehicle systems, access controls, or other third-party equipment.
- Battery runtime, radio range, capture rate, and hardware reliability have not been measured for this repository.

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
<img width="362" height="241" alt="image" src="https://github.com/user-attachments/assets/117580a4-0350-4420-b1ea-d30108fdf92c" />
 
 <img width="336" height="234" alt="image" src="https://github.com/user-attachments/assets/ab355bba-99f9-4e99-8742-ce3222dac94e" />

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

## Software setup

The repository contains two firmware forms. The checked-in filenames and layout are not a standard PlatformIO source tree, so use one of the local-copy workflows below. No tracked files need to be renamed or moved.

### Option A — Arduino IDE single-file sketch

1. Create a local sketch folder named `rf_interceptor`.
2. Copy `rf_interceptor(for ardino uno).ino` into it as `rf_interceptor.ino`. This is a local working copy; despite the original filename, the sketch targets an **ESP32**, not an Arduino Uno.
3. Install ESP32 board support from Espressif Systems and install the `RF24` library by TMRh20.
4. Select an ESP32 board matching your hardware (the guide assumes **ESP32 Dev Module**) and the correct serial port.
5. Compile and upload, then open Serial Monitor at **115200 baud**.

### Option B — PlatformIO multi-file firmware

The root `platformio.ini` declares the build environment, but the multi-file source and headers are currently at the repository root. PlatformIO's default build expects source files under `src/`, so the checked-in layout is not ready for a direct root-level build.

In a separate local build copy, keep `platformio.ini` at the root and place these files under `src/`:

- `main.cpp`
- `config.h`
- `nrf_handler.cpp` and `nrf_handler.h`
- `ble_server.cpp` and `ble_server.h`

Then run `pio run` to build, `pio run --target upload` to flash, and `pio device monitor -b 115200` to view serial output. These steps describe the expected PlatformIO layout; the firmware was not compiled during this review.

## How to Use It

### Basic Operation

1. Turn on the device using the toggle switch
2. Open Arduino IDE and go to Tools > Serial Monitor
3. Set the baud rate to 115200
4. You will start seeing channel scan results immediately

### Web Interface

Serve the root-level `index.html` from a local HTTP server (for example, run `python -m http.server 8000` in a local copy and open `http://localhost:8000/`). The page uses Web Serial, so use a compatible Chromium-based browser and grant access to the ESP32 serial port. This file connects to the ESP32 over Web Serial (Chrome and Edge only) and shows you a live channel activity map. You can see which channels have traffic and view the raw packet bytes.

### LED Status Indicators

If you add an LED to GPIO 2 (the built-in LED on most ESP32 boards), it will blink once when a packet is captured and stay solid when the device is scanning and idle.

---

## Actual repository structure

All of the following files are at the repository root:

| File | Purpose |
|---|---|
| `main.cpp` | Multi-file firmware entry point |
| `config.h` | Pin, channel, dwell-time, and BLE settings for the multi-file version |
| `nrf_handler.cpp`, `nrf_handler.h` | NRF24 scanner/transmitter logic |
| `ble_server.cpp`, `ble_server.h` | BLE service and packet notifications |
| `rf_interceptor(for ardino uno).ino` | Separate single-file ESP32 sketch; the filename is misleading |
| `platformio.ini` | PlatformIO environment and declared libraries |
| `index.html` | Browser interface using Web Serial |
| `wiring.md` | Wiring guide |
| `parts_list.md` | Parts list |
| `how_it_works.md` | Protocol and design explanation |
| `TROUBLESHOOTING.md` | Troubleshooting notes |
| `LICENSE` | License text |

There are no `src/`, `hardware/`, or `demo/` directories in the current tree. Use the actual root-level paths above.

## Troubleshooting

- **NRF module not detected:** check CE/CSN wiring, 3.3 V supply stability, common ground, and the capacitor placement described in `wiring.md`.
- **Repeated resets:** the PA/LNA radios can have current spikes. Verify the supply can handle the modules and keep decoupling capacitors close to their power pins.
- **No full packets:** see the address-filtering limitation above and `how_it_works.md`; do not interpret carrier activity as decoded traffic.
- **Garbled serial output:** use 115200 baud.
- **Browser cannot connect:** use a compatible Chromium-based browser, serve `index.html` from localhost, and grant Web Serial access to the correct port.

## Authorized-use and safety notice

Use the device only for controlled testing of radios and receivers you own or are authorized to assess. Do not capture or replay communications to interfere with other people's devices or bypass access-control systems. Follow local radio and privacy laws. The project is experimental and has not been validated as a security-testing instrument.

## License

See the root-level `LICENSE` file.

## Skills & technologies

`Arduino/C++` · `HTML/CSS` · `ESP32` · `NRF24L01` · `BLE`

## How it works

The multi-file implementation starts at `main.cpp`; the separate Arduino IDE implementation is `rf_interceptor(for ardino uno).ino`. The scanner uses carrier detection and address-filtered NRF24 reception across the configured channel range. The implementation is not a promiscuous Wi-Fi/Bluetooth receiver. Review `how_it_works.md` for the project's stated design details.

## Maintenance & Audit

- **Audit date:** 11 November 2026
- **Documentation status:** README reviewed and audit record added on this date.
- **Scope and evidence:** This is a documentation-level maintenance pass. It does not establish that source code compiles, dependencies are current, security has been tested, or hardware/runtime behavior has been verified.
- **Code and tests:** No new functional code changes or test results are claimed by this README update. Run project-specific build, test, and hardware/runtime checks before relying on the project.
- **Maintenance baseline:** Documentation status recorded through 11 November 2026; revisit setup instructions, dependencies, known limitations, and tests when the project changes.

## Maintenance & Audit

- **Audit date:** 11 November 2026
- **Documentation status:** README reviewed and audit record added on this date.
- **Scope and evidence:** This is a documentation-level maintenance pass. It does not establish that source code compiles, dependencies are current, security has been tested, or hardware/runtime behavior has been verified.
- **Code and tests:** No new functional code changes or test results are claimed by this README update. Run project-specific build, test, and hardware/runtime checks before relying on the project.
- **Maintenance baseline:** Documentation status recorded through 11 November 2026; revisit setup instructions, dependencies, known limitations, and tests when the project changes.
