# Troubleshooting Guide

This guide covers every common problem people run into when building this project for the first time. Work through the relevant section step by step before asking for help online.

---

## Table of Contents

1. NRF module not detected at startup
2. ESP32 keeps restarting or crashing
3. No packets being captured
4. Serial monitor shows garbled text
5. Bluetooth not showing up on phone
6. Upload fails in Arduino IDE
7. Capacitor and power problems

---

## 1. NRF Module Not Detected at Startup

The serial monitor says something like:
`[ERROR] Scanner module (NRF #1) not found`

This is the most common beginner problem and almost always comes down to one of four things.

**Check 1: Wrong wiring**

Go through every single wire between your ESP32 and the NRF module one at a time. Do not assume it is correct. Use the wiring table in README.md and verify each pin physically.

Common mistakes:
- Swapping MOSI and MISO (they look similar in the pin name)
- Using 5V instead of 3.3V for the NRF module (this will damage it)
- CE and CSN swapped between the two modules

**Check 2: Missing or wrong capacitor**

The NRF24L01 PA+LNA module pulls large current spikes when it powers up. Without a capacitor, the voltage dips and the module fails to initialize. Place a 100uF electrolytic capacitor across the VCC and GND pins of the NRF module. The positive leg (the longer one) goes to VCC. If you do not have 100uF, two 47uF capacitors in parallel will also work.

**Check 3: Bad jumper wire or loose connection**

If you are on a breadboard, push every wire firmly into its hole. Try replacing the MISO and MOSI wires first as they are the most commonly faulty.

**Check 4: Defective NRF module**

Some cheap NRF24L01 modules are faulty from the factory. If you have a second module, try swapping it in.

---

## 2. ESP32 Keeps Restarting or Crashing

If the ESP32 keeps rebooting in a loop, the first thing to check is power.

**The PA+LNA version of the NRF24L01 draws up to 115mA each when transmitting.** Two modules together can draw 230mA just for the radios, plus the ESP32 itself needs up to 240mA. That is potentially 470mA total. Many USB cables and cheap power banks cannot sustain this.

What to try:

- Use a USB cable that is rated for data and charging (not a charge-only cable)
- Plug into a wall adapter instead of a laptop USB port
- Add a larger capacitor (470uF or 1000uF) across the 3.3V and GND rails on your breadboard
- If using a LiPo, check that the TP4056 protection module is the type with protection circuit, not just a bare charger (the module should have 4 components on the board, not 2)

If the ESP32 crashes and the serial monitor shows a stack trace, copy the full crash log and search for the function names listed. They will point you to the exact line of code that caused the problem.

---

## 3. No Packets Being Captured

The device is running and scanning but never prints a packet line.

**Reason 1: Nothing nearby to capture**

The scanner looks for NRF24-based devices, not regular Bluetooth devices. If you do not have another NRF device nearby, there will be nothing to find. To test that scanning is working, run two of these boards near each other, or bring a wireless mouse or keyboard (many use NRF24 or compatible chips).

**Reason 2: Dwell time too short**

The DWELL_TIME_US value in config.h controls how long the scanner pauses on each channel. If it is set too low (below 128 microseconds), it will miss many packets. Try setting it to 512 or even 1000 and see if that helps.

**Reason 3: Data rate mismatch**

The scanner defaults to 2Mbps. If the device you are trying to capture uses 250kbps or 1Mbps, you need to change RF24_2MBPS to RF24_250KBPS or RF24_1MBPS in nrf_handler.cpp. You may need to try each rate one at a time.

**Reason 4: Address mismatch**

The NRF24 hardware filters out packets that do not match its address. We use address 0xABCDABCD71 in the code. This means we can only catch packets sent to that address. To do wider scanning, you need to change the address or use a different scanning technique (see docs/how_it_works.md for more on this).

---

## 4. Serial Monitor Shows Garbled Text

Characters like `?????` or random symbols appear in the serial monitor instead of readable text.

The baud rate is wrong. In the Arduino IDE serial monitor, there is a dropdown in the bottom right corner. Make sure it says 115200. The code initializes the serial port at 115200 baud and if your monitor is set to a different speed, every character comes out wrong.

---

## 5. Bluetooth Not Showing Up on Phone

The device is running but you cannot see "RF-Interceptor" when you scan for Bluetooth devices on your phone.

**Check 1: BLE vs Classic Bluetooth**

This device uses BLE (Bluetooth Low Energy), also called Bluetooth Smart. Make sure your phone's Bluetooth scanner is looking for BLE devices, not just classic Bluetooth. Most phones see BLE by default in their settings, but some Bluetooth scanner apps need BLE mode turned on specifically.

**Check 2: The module may have already connected to something else**

If another device connected first, the ESP32 will stop advertising. Disconnect all Bluetooth devices and reset the ESP32 by pressing its reset button.

**Check 3: The BLE init may have failed**

Open the serial monitor and look for the line `[OK] Bluetooth name: RF-Interceptor`. If you do not see this line, the BLE stack failed to start. This sometimes happens when the flash memory is nearly full. Try erasing the ESP32 flash: in Arduino IDE, go to Tools > Erase Flash > All Flash Contents, then re-upload.

---

## 6. Upload Fails in Arduino IDE

Arduino IDE shows an error like `A fatal error occurred: Failed to connect to ESP32`.

**Step 1: Hold the BOOT button**

Most ESP32 boards have a button labeled BOOT or IO0. When the Arduino IDE says "Connecting..." in the output, hold this button down for 2 seconds. Release it once you see dots appearing on the screen. This puts the ESP32 into programming mode.

**Step 2: Wrong COM port**

Make sure you selected the right port in Tools > Port. If you are not sure which one is your ESP32, unplug it, check what ports are listed, plug it back in, and see which new port appeared.

**Step 3: Driver not installed**

Some ESP32 boards use a CH340 or CP2102 USB chip that requires a driver on Windows. Search for "CH340 driver" or "CP2102 driver" depending on which chip your board uses (it is usually printed on the underside of the board near the USB connector).

**Step 4: Wrong board selected**

Go to Tools > Board > esp32 > ESP32 Dev Module. If your board is a different variant (Wemos D1 Mini ESP32, NodeMCU-32S, etc.), select the matching entry.

---

## 7. Capacitor and Power Problems

**The NRF module gets hot**

You are powering it from 5V instead of 3.3V. The NRF24L01 is a 3.3V device. Connect VCC to the 3V3 pin on the ESP32, not the 5V pin. Powering it from 5V will damage the module permanently.

**The project works on USB but not on battery**

Your LiPo is either too small or the TP4056 module has a current limit set too low. A 1000mAh 3.7V LiPo should be the absolute minimum. The TP4056 output should be connected through a protection circuit before going to the ESP32 VIN pin. Check that the TP4056 module has a DW01 or similar protection chip on it. Without protection, a low battery can damage both the battery and the TP4056.

**Battery drains very fast**

The PA+LNA modules consume a lot of power. Battery life on a 1000mAh pack is typically 3 to 5 hours of continuous scanning. To extend battery life, increase DWELL_TIME_US to slow down the scan loop, or modify the code to put the ESP32 into light sleep between scans (see docs/how_it_works.md).

---

## Still Stuck?

If you have checked everything in this guide and the problem persists:

1. Open a GitHub Issue in this repository and describe the problem in detail
2. Include the full serial monitor output from the moment of power-on
3. Take a clear photo of your wiring and include it in the issue
4. Mention what NRF24 modules you have (with or without external antenna, brand if known)

Most problems can be diagnosed from a good photo and the serial log.
